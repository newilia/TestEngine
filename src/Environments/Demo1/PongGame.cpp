#include "PongGame.h"

#include "Engine/Behaviour/ButtonBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/ConvexShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "Environments/Pong/AiPlatformControllerBehaviour.h"
#include "Environments/Pong/PongBall.h"
#include "Environments/Pong/PongPlayfield.h"
#include "Environments/Pong/UserPlatformControllerBehaviour.h"

#include <fmt/format.h>

#include <cmath>
#include <memory>

using std::make_shared;
using std::shared_ptr;

namespace Demo1 {

	namespace {

		constexpr float bodiesRestitution = 1;
		constexpr int kPongScoreFontSize = 140;
		constexpr int kTapPromptFontSize = 56;

		shared_ptr<SceneNode> sUserPlatform;
		shared_ptr<SceneNode> sAiPlatform;
		shared_ptr<PongBall> sBall;

		int userScore = 0;
		int aiScore = 0;
		shared_ptr<TextVisual> scoreText;
		shared_ptr<SceneNode> scoreboardNode;

		bool sAwaitingFirstTap = false;
		std::weak_ptr<SceneNode> sPongGameRoot;
		float sBallRadius = 0.f;

		void OnScoreboardTap(const sf::Event&);

		void ConfigureNewPongSession(float ballRadius) {
			sAwaitingFirstTap = true;
			sBallRadius = ballRadius;
		}

		void RegisterPongGameRoot(const shared_ptr<SceneNode>& root) {
			sPongGameRoot = root;
		}

		float FieldHalfHeight() {
			return GetPongPlayfieldRect().size.y * 0.5f;
		}

		sf::Vector2f InitialBallPositionLocal() {
			return {0.f, 0.f};
		}

		sf::Vector2f InitialBallVelocity() {
			return {0, 500};
		}

		sf::Vector2f FirstServeBallVelocity() {
			return {0.f, -500.f};
		}

		sf::Vector2f InitialUserPlatformPositionLocal() {
			const float hy = FieldHalfHeight();
			return {0.f, hy - 100.f};
		}

		sf::Vector2f InitialAiPlatformPositionLocal() {
			const float hy = FieldHalfHeight();
			return {0.f, -hy + 100.f};
		}

		void ResetRound() {
			if (!sBall || !sUserPlatform || !sAiPlatform) {
				return;
			}

			sBall->GetNode()->GetLocalTransform()->SetPosition(InitialBallPositionLocal());
			auto ballRb = sBall->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
			ballRb->SetVelocity(InitialBallVelocity());
			ballRb->SetAngularSpeed(0.f);

			sUserPlatform->GetLocalTransform()->SetPosition(InitialUserPlatformPositionLocal());
			if (auto u = sUserPlatform->FindBehaviour<UserPlatformControllerBehaviour>()) {
				u->ResyncSpawnFromNode();
			}

			sAiPlatform->GetLocalTransform()->SetPosition(InitialAiPlatformPositionLocal());
			if (auto ai = sAiPlatform->FindBehaviour<AiPlatformControllerBehaviour>()) {
				ai->ResyncSpawnFromNode();
				ai->ClearPendingObservations();
			}
		}

		void UpdateScoreText() {
			if (!scoreText || !scoreboardNode) {
				return;
			}
			scoreText->SetString(fmt::format("{}:{}", userScore, aiScore));
			const auto lb = scoreText->GetLocalBounds();
			scoreText->SetOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
			scoreboardNode->GetLocalTransform()->SetPosition({0.f, 0.f});
		}

		void OnLose() {
			++aiScore;
			UpdateScoreText();
			ResetRound();
		}

		void OnWin() {
			++userScore;
			UpdateScoreText();
			ResetRound();
		}

		shared_ptr<SceneNode> CreateDefaultPlatform(sf::Vector2f size, float rotationDeg, sf::Color color) {
			auto node = make_shared<SceneNode>();
			node->SetName("Platform");
			auto convexShape = std::make_shared<ConvexShapeVisual>();
			node->SetVisual(convexShape);

			constexpr int pointCount = 42;
			constexpr float curvature = 0.9f;

			auto shape = convexShape->GetShape();
			shape->setPointCount(pointCount);
			for (int i = 0; i < pointCount; ++i) {
				float x = (static_cast<float>(i) / (pointCount - 1) - 0.5f) * 2.0f;
				float y = sqrt(1 - x * x * curvature);
				sf::Vector2f point(x * size.x * 0.5f, y * size.y);
				shape->setPoint(i, point);
			}
			shape->setRotation(sf::degrees(rotationDeg));
			shape->setOrigin(Utils::FindCenterOfMass(shape));
			shape->setFillColor(color);

			auto physicsBody = node->RequireBehaviour<PhysicsBodyBehaviour>();
			physicsBody->GetInteractionGroups().set(1, true);
			physicsBody->SetImmovable(true);
			physicsBody->SetRestitution(bodiesRestitution);

			return node;
		}

		shared_ptr<SceneNode> MakeMovementBoundsRect(const char* name, sf::Vector2f size) {
			auto node = make_shared<SceneNode>();
			node->SetName(name);
			auto rv = make_shared<RectangleShapeVisual>();
			auto* sh = rv->GetShape();
			sh->setSize(size);
			sh->setOrigin({size.x * 0.5f, size.y * 0.5f});
			sh->setFillColor(sf::Color::Transparent);
			node->SetVisual(std::move(rv));
			auto sorting = make_shared<RelativeSortingStrategy>();
			sorting->SetPriority(-5);
			node->SetSortingStrategy(std::move(sorting));
			return node;
		}

		struct MovementBoundNodes
		{
			shared_ptr<SceneNode> userStrip;
			shared_ptr<SceneNode> aiStrip;
		};

		MovementBoundNodes CreateMovementBoundRects(SceneNode* root) {
			const sf::Vector2f sz = GetPongPlayfieldRect().size;
			const float hy = sz.y * 0.5f;
			const float screenH = GetPongWindowSize().y;
			const float stripH = kPongPlatformVerticalStripScreenFraction * screenH;

			MovementBoundNodes m;
			m.userStrip = MakeMovementBoundsRect("PongBounds_UserStrip", {sz.x, stripH});
			m.aiStrip = MakeMovementBoundsRect("PongBounds_AiStrip", {sz.x, stripH});

			root->AddChild(m.userStrip);
			m.userStrip->GetLocalTransform()->SetPosition({0.f, hy - stripH * 0.5f});

			root->AddChild(m.aiStrip);
			m.aiStrip->GetLocalTransform()->SetPosition({0.f, -hy + stripH * 0.5f});

			return m;
		}

		void AddBall(SceneNode* root, float radius, const sf::Vector2f& initialVelocity) {
			constexpr float pointsCountConstant = 3.f;
			constexpr float speedDampingFactor = 0.1f;
			const sf::Color color(40, 170, 255, 200);
			const sf::Vector2f vel = initialVelocity;

			auto ballNode = make_shared<SceneNode>();
			auto circleVisual = std::make_shared<CircleShapeVisual>();
			ballNode->SetVisual(circleVisual);
			auto ball = make_shared<PongBall>(ballNode);
			ball->SetupBehaviours();
			ball->SetMaxSpeed(400.f);
			ball->SetSpeedDampingFactor(speedDampingFactor);
			auto shape = circleVisual->GetShape();
			ball->GetNode()->SetName("Ball");

			shape->setRadius(radius);
			auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
			shape->setPointCount(pointsCount);
			auto outlineColor = color;
			outlineColor.a = 255;

			shape->setFillColor(color);
			shape->setOutlineColor(outlineColor);
			shape->setOutlineThickness(1);
			shape->setOrigin(Utils::FindCenterOfMass(shape));

			auto rigidBody = ball->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
			rigidBody->SetMass(3.14f * radius * radius);
			rigidBody->SetRestitution(bodiesRestitution);
			rigidBody->SetVelocity(vel);
			rigidBody->GetInteractionGroups().set(0, true);
			rigidBody->GetInteractionGroups().set(1, true);
			rigidBody->GetOverlappingGroups().set(0, true);
			rigidBody->SetGravityScale(0.f);

			root->AddChild(ball->GetNode());
			ballNode->GetLocalTransform()->SetPosition(InitialBallPositionLocal());

			ballNode->NotifyLifecycleInitRecursive();

			sBall = ball;
		}

		void AddWalls(SceneNode* root, float wallThickness) {
			const sf::Vector2f sz = GetPongPlayfieldRect().size;
			const float hx = sz.x * 0.5f;
			const float hy = sz.y * 0.5f;
			const float t = wallThickness;

			const sf::Vector2f horizSize{sz.x + 2.f * t, t};
			const sf::Vector2f vertSize{t, sz.y + 2.f * t};

			std::string wallNames[] = {"bottom", "top", "left", "right"};
			sf::Vector2f wallSizes[] = {horizSize, horizSize, vertSize, vertSize};

			const sf::Vector2f wallCentersLocal[] = {
			    {0.f, hy + t * 0.5f}, {0.f, -hy - t * 0.5f}, {-hx - t * 0.5f, 0.f}, {hx + t * 0.5f, 0.f}};
			for (int i = 0; i < 4; ++i) {
				auto wallNode = make_shared<SceneNode>();
				wallNode->SetName(wallNames[i]);
				auto rectVisual = std::make_shared<RectangleShapeVisual>();
				wallNode->SetVisual(rectVisual);

				auto rectShape = rectVisual->GetShape();
				rectShape->setSize(wallSizes[i]);
				rectShape->setOrigin(Utils::FindCenterOfMass(rectShape));

				root->AddChild(wallNode);
				wallNode->GetLocalTransform()->SetPosition(wallCentersLocal[i]);

				auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
				bodyBeh->SetImmovable(true);
				bodyBeh->SetRestitution(bodiesRestitution);

				if (i < 2) {
					rectShape->setFillColor(sf::Color(200, 200, 200, 50));
					bodyBeh->GetOverlappingGroups().set(0, true);

					if (i == 0) {
						[[maybe_unused]] auto connection =
						    bodyBeh->GetOnOverlapSignal().Connect([](const IntersectionDetails&) {
							    OnLose();
						    });
					}
					else {
						[[maybe_unused]] auto connection =
						    bodyBeh->GetOnOverlapSignal().Connect([](const IntersectionDetails&) {
							    OnWin();
						    });
					}
				}
				else {
					rectShape->setFillColor(sf::Color(200, 200, 200, 255));
					bodyBeh->GetInteractionGroups().set(0, true);
				}
			}
		}

		void AddUserPlatform(SceneNode* root, sf::Vector2f platformSize) {
			const sf::Color color(220, 220, 20);

			auto platformNode = CreateDefaultPlatform(platformSize, 180.f, color);
			platformNode->SetName("User_platform");

			constexpr float velFactor = 50.f;
			const sf::Vector2f maxSpeed = {15000.f, 1000.f};

			auto userBehaviour = std::make_shared<UserPlatformControllerBehaviour>();
			platformNode->AddBehaviour(userBehaviour);
			userBehaviour->_velLimit = maxSpeed;
			userBehaviour->_speedFactor = velFactor;

			root->AddChild(platformNode);
			platformNode->GetLocalTransform()->SetPosition(InitialUserPlatformPositionLocal());

			sUserPlatform = platformNode;
		}

		void AddAiPlatform(SceneNode* root, sf::Vector2f platformSize) {
			const sf::Color color(220, 220, 20);

			auto platformNode = CreateDefaultPlatform(platformSize, 0.f, color);
			platformNode->SetName("AI_platform");

			auto aiBehaviour = std::make_shared<AiPlatformControllerBehaviour>();
			aiBehaviour->BeginObserve(sUserPlatform, sBall);
			platformNode->AddBehaviour(aiBehaviour);

			constexpr float velFactor = 50.f;
			const sf::Vector2f maxSpeed = {3000.f, 1000.f};

			aiBehaviour->_velLimit = maxSpeed;
			aiBehaviour->_speedFactor = velFactor;
			aiBehaviour->SetObservePeriod(sf::milliseconds(10));
			aiBehaviour->SetReactionDelay(sf::milliseconds(100));

			root->AddChild(platformNode);
			platformNode->GetLocalTransform()->SetPosition(InitialAiPlatformPositionLocal());

			sAiPlatform = platformNode;
		}

		void AddScoreboard(SceneNode* root) {
			auto* font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont();
			if (!font) {
				return;
			}

			userScore = 0;
			aiScore = 0;

			auto node = std::make_shared<SceneNode>();
			auto sorting = std::make_shared<RelativeSortingStrategy>();
			sorting->SetPriority(-1);
			node->SetSortingStrategy(sorting);
			node->SetName("Score");

			auto textVisual = std::make_shared<TextVisual>();
			textVisual->Init(*font, "Tap to play", kTapPromptFontSize);
			textVisual->SetFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(0.3f * 255.f)));
			const auto lb = textVisual->GetLocalBounds();
			textVisual->SetOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});

			scoreText = textVisual;
			node->SetVisual(std::move(textVisual));

			scoreboardNode = node;

			auto button = std::make_shared<ButtonBehaviour>();
			node->AddBehaviour(button);
			[[maybe_unused]] auto tapConn = button->SubscribeOnTap(OnScoreboardTap);

			root->AddChild(std::move(node));
			scoreboardNode->GetLocalTransform()->SetPosition({0.f, 0.f});
		}

		void OnScoreboardTap(const sf::Event&) {
			if (!sAwaitingFirstTap) {
				return;
			}
			auto root = sPongGameRoot.lock();
			if (!root || !scoreText) {
				return;
			}
			sAwaitingFirstTap = false;
			AddBall(root.get(), sBallRadius, FirstServeBallVelocity());
			if (auto ai = sAiPlatform->FindBehaviour<AiPlatformControllerBehaviour>()) {
				ai->BeginObserve(sUserPlatform, sBall);
			}
			scoreText->SetCharacterSize(kPongScoreFontSize);
			UpdateScoreText();
		}

	} // namespace

	std::shared_ptr<SceneNode> CreatePongGameNode(float fieldWidth, float fieldHeight, float platformWidth,
	                                              float platformHeight, float wallThickness, float ballRadius) {
		if (fieldWidth <= 0.f || fieldHeight <= 0.f || platformWidth <= 0.f || platformHeight <= 0.f ||
		    wallThickness <= 0.f) {
			return nullptr;
		}
		ConfigureNewPongSession(ballRadius);
		auto* window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return nullptr;
		}
		const sf::Vector2f winSz = sf::Vector2f(window->getSize());
		const sf::Vector2f fieldCenter{winSz.x * 0.75f, winSz.y * 0.5f};
		const sf::Vector2f fieldPos{fieldCenter.x - fieldWidth * 0.5f, fieldCenter.y - fieldHeight * 0.5f};
		SetPongPlayfieldRectOverride(sf::FloatRect(fieldPos, {fieldWidth, fieldHeight}));

		auto root = make_shared<SceneNode>();
		root->SetName("Pong");
		root->SetPosGlobal(GetPongPlayfieldRect().getCenter());
		RegisterPongGameRoot(root);

		const sf::Vector2f platformSize{platformWidth, platformHeight};

		AddWalls(root.get(), wallThickness);
		const MovementBoundNodes movementBounds = CreateMovementBoundRects(root.get());
		AddUserPlatform(root.get(), platformSize);
		AddAiPlatform(root.get(), platformSize);
		if (auto u = sUserPlatform->FindBehaviour<UserPlatformControllerBehaviour>()) {
			u->SetMovementBounds(movementBounds.userStrip);
		}
		if (auto ai = sAiPlatform->FindBehaviour<AiPlatformControllerBehaviour>()) {
			ai->SetMovementBounds(movementBounds.aiStrip);
		}
		AddScoreboard(root.get());

		return root;
	}

} // namespace Demo1
