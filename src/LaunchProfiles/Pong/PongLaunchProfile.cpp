#include "PongLaunchProfile.h"

#include "AiPlatformControllerBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/ConvexShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "PongBall.h"
#include "PongPlayfield.h"
#include "UserPlatformControllerBehaviour.h"

#include <fmt/format.h>

using std::make_shared;
using std::shared_ptr;

constexpr float bodiesRestitution = 1;
constexpr float wallActualWidth = 200;
constexpr float wallVisibleWidth = 10;
static shared_ptr<SceneNode> sUserPlatform;
static shared_ptr<SceneNode> sAiPlatform;
static shared_ptr<PongBall> sBall;

namespace {

	sf::Vector2f InitialBallPosition() {
		return GetPongPlayfieldRect().getCenter();
	}

	sf::Vector2f InitialBallVelocity() {
		return {0, 500};
	}

	sf::Vector2f InitialUserPlatformPosition() {
		const auto field = GetPongPlayfieldRect();
		return {field.getCenter().x, field.position.y + field.size.y - wallVisibleWidth - 100};
	}

	sf::Vector2f InitialAiPlatformPosition() {
		const auto field = GetPongPlayfieldRect();
		return {field.getCenter().x, field.position.y + wallVisibleWidth + 100};
	}

} // namespace

constexpr int kPongScoreFontSize = 140;

PongLaunchProfile::~PongLaunchProfile() = default;

void PongLaunchProfile::Setup() {
	auto& mainContext = Engine::MainContext::GetInstance();
	auto videoMode = sf::VideoMode::getDesktopMode();
	mainContext.CreateMainWindow(videoMode, "Pong");
	mainContext.GetMainWindow()->setMouseCursorVisible(false);
	mainContext.GetPhysicsProcessor()->SetGravityEnabled(false);
	mainContext.SetScene(BuildScene());
	Utils::MaximizeWindow(*mainContext.GetMainWindow());
	SubscribeForEvents();
}

void PongLaunchProfile::OnEvent(const sf::Event& event) {
	if (auto e = event.getIf<sf::Event::KeyPressed>()) {
		auto& mainContext = Engine::MainContext::GetInstance();
		if (e->code == sf::Keyboard::Key::R) {
			mainContext.SetScene(BuildScene());
		}
		else if (e->code == sf::Keyboard::Key::Escape) {
			std::exit(EXIT_SUCCESS);
		}
	}
}

void PongLaunchProfile::AddBall(Scene* scene, float radius) {
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = InitialBallVelocity();
	const sf::Vector2f pos = InitialBallPosition();

	auto ballNode = SceneNode::Create();
	ballNode->SetName("Ball");

	auto ball = make_shared<PongBall>(ballNode);
	ball->SetupBehaviours();
	ball->SetMaxSpeed(400.f);
	ball->SetSpeedDampingFactor(speedDampingFactor);

	auto circleVisual = ballNode->RequireVisual<CircleShapeVisual>();

	circleVisual->SetRadius(radius);
	auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
	circleVisual->SetPointCount(pointsCount);
	auto outlineColor = color;
	outlineColor.a = 255;

	circleVisual->SetFillColor(color);
	circleVisual->SetOutlineColor(outlineColor);
	circleVisual->SetOutlineThickness(1);
	circleVisual->SetOrigin(circleVisual->GetLocalBounds().getCenter());
	Utils::SetLocalPosToWorld(ballNode, pos);

	auto rigidBody = ball->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
	rigidBody->SetMass(3.14f * radius * radius);
	rigidBody->SetRestitution(bodiesRestitution);
	rigidBody->SetVelocity(vel);
	rigidBody->GetCollisionGroups().set(0, true);
	rigidBody->GetCollisionGroups().set(1, true);
	rigidBody->GetOverlapGroups().set(0, true);
	scene->GetRoot()->AddChild(ball->GetNode());

	sBall = ball;
}

shared_ptr<SceneNode> PongLaunchProfile::CreateDefaultPlatform(
    sf::Vector2f size, sf::Vector2f pos, float rotationDeg, sf::Color color) const {
	auto node = SceneNode::Create();
	node->SetName("Platform");
	auto convexShape = std::make_shared<ConvexShapeVisual>();
	node->SetVisual(convexShape);

	constexpr int pointCount = 42;
	constexpr float curvature = 0.9f;

	auto shape = convexShape->GetShape();
	shape->setPointCount(pointCount);
	// half-ellipse shape
	for (int i = 0; i < pointCount; ++i) {
		float x = (static_cast<float>(i) / (pointCount - 1) - 0.5f) * 2.0f;
		float y = sqrt(1 - x * x * curvature);
		sf::Vector2f point(x * size.x * 0.5f, y * size.y);
		shape->setPoint(i, point);
	}
	shape->setRotation(sf::degrees(rotationDeg));
	shape->setOrigin(Utils::FindCenterOfMass(shape));
	Utils::SetLocalPosToWorld(node, pos);
	shape->setFillColor(color);

	auto physicsBody = node->RequireBehaviour<PhysicsBodyBehaviour>();
	physicsBody->SetFixed(true);
	physicsBody->SetRestitution(bodiesRestitution);

	return node;
}

void PongLaunchProfile::AddWalls(Scene* scene) {
	const auto field = GetPongPlayfieldRect();
	const sf::Vector2f o = field.position;
	const sf::Vector2f sz = field.size;

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {
	    {sz.x, wallActualWidth}, {sz.x, wallActualWidth}, {wallActualWidth, sz.y}, {wallActualWidth, sz.y}};

	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	sf::Vector2f wallPositions[] = {{o.x + sz.x / 2, o.y + sz.y + wallOffset}, {o.x + sz.x / 2, o.y - wallOffset},
	    {o.x - wallOffset, o.y + sz.y / 2}, {o.x + sz.x + wallOffset, o.y + sz.y / 2}};
	for (int i = 0; i < 4; ++i) {
		auto wallNode = SceneNode::Create();
		wallNode->SetName(wallNames[i]);
		auto rectVisual = std::make_shared<RectangleShapeVisual>();
		wallNode->SetVisual(rectVisual);

		auto rectShape = rectVisual->GetShape();
		rectShape->setSize(wallSizes[i]);
		rectShape->setOrigin(Utils::FindCenterOfMass(rectShape));
		Utils::SetLocalPosToWorld(wallNode, wallPositions[i]);

		auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->SetFixed(true);
		bodyBeh->SetRestitution(bodiesRestitution);

		if (i < 2) {
			rectShape->setFillColor(sf::Color(200, 200, 200, 50));
			bodyBeh->GetOverlapGroups().set(0, true);

			if (i == 0) {
				Subscribe(bodyBeh->GetOnOverlapSignal(), [this](const IntersectionDetails&) {
					OnLose();
				});
			}
			else {
				Subscribe(bodyBeh->GetOnOverlapSignal(), [this](const IntersectionDetails&) {
					OnWin();
				});
			}
		}
		else {
			rectShape->setFillColor(sf::Color(200, 200, 200, 255));
			bodyBeh->GetCollisionGroups().set(0, true);
		}
		scene->GetRoot()->AddChild(std::move(wallNode));
	}
}

void PongLaunchProfile::AddUserPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const sf::Vector2f pos = InitialUserPlatformPosition();
	const sf::Color color(220, 220, 20);

	auto platformNode = CreateDefaultPlatform(size, pos, 180.f, color);
	platformNode->SetName("User_platform");

	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = {15000.f, 1000.f};

	auto userBehaviour = std::make_shared<UserPlatformControllerBehaviour>();
	platformNode->AddBehaviour(userBehaviour);
	userBehaviour->SetVelLimit(maxSpeed);
	userBehaviour->SetSpeedFactor(velFactor);

	scene->GetRoot()->AddChild(platformNode);

	sUserPlatform = platformNode;
}

void PongLaunchProfile::AddAiPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const sf::Vector2f pos = InitialAiPlatformPosition();
	const sf::Color color(220, 220, 20);

	auto platformNode = CreateDefaultPlatform(size, pos, 0.f, color);
	platformNode->SetName("AI_platform");

	auto aiBehaviour = std::make_shared<AiPlatformControllerBehaviour>();
	aiBehaviour->BeginObserve(sUserPlatform, sBall);
	platformNode->AddBehaviour(aiBehaviour);

	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = {3000.f, 1000.f};

	aiBehaviour->SetVelLimit(maxSpeed);
	aiBehaviour->SetSpeedFactor(velFactor);
	aiBehaviour->SetObservePeriod(sf::milliseconds(10));
	aiBehaviour->SetReactionDelay(sf::milliseconds(100));

	scene->GetRoot()->AddChild(platformNode);

	sAiPlatform = platformNode;
}

void PongLaunchProfile::AddScoreboard(Scene* scene) {
	auto* font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont();
	if (!font) {
		return;
	}

	_userScore = 0;
	_aiScore = 0;

	auto node = SceneNode::Create();
	auto sorting = std::make_shared<RelativeSortingStrategy>();
	sorting->SetPriority(-1);
	node->SetSortingStrategy(sorting);
	node->SetName("Score");

	_scoreText = std::make_shared<TextVisual>();
	_scoreText->Init(*font, "0:0", kPongScoreFontSize);
	_scoreText->SetFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(0.3f * 255.f)));

	const sf::Vector2f center = GetPongPlayfieldRect().getCenter();
	const auto lb = _scoreText->GetLocalBounds();
	_scoreText->SetOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});

	node->SetVisual(std::shared_ptr<TextVisual>(_scoreText));
	Utils::SetLocalPosToWorld(node, center);
	_scoreboardNode = node;
	scene->GetRoot()->AddChild(std::move(node));
}

void PongLaunchProfile::UpdateScoreText() {
	if (!_scoreText || !_scoreboardNode) {
		return;
	}
	const auto text = fmt::format("{}:{}", _userScore, _aiScore);
	_scoreText->SetString(text);
	const auto lb = _scoreText->GetLocalBounds();
	_scoreText->SetOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
	Utils::SetLocalPosToWorld(_scoreboardNode, GetPongPlayfieldRect().getCenter());
}

std::shared_ptr<Scene> PongLaunchProfile::BuildScene() {
	ClearPongPlayfieldRectOverride();
	auto scene = make_shared<Scene>();

	AddWalls(scene.get());
	AddBall(scene.get(), 20);
	AddUserPlatform(scene.get());
	AddAiPlatform(scene.get());
	AddScoreboard(scene.get());
	return scene;
}

void PongLaunchProfile::OnLose() {
	++_aiScore;
	UpdateScoreText();
	ResetRound();
}

void PongLaunchProfile::OnWin() {
	++_userScore;
	UpdateScoreText();
	ResetRound();
}

void PongLaunchProfile::ResetRound() {
	if (!sBall || !sUserPlatform || !sAiPlatform) {
		return;
	}

	Utils::SetLocalPosToWorld(sBall->GetNode(), InitialBallPosition());
	auto ballRb = sBall->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
	ballRb->SetVelocity(InitialBallVelocity());
	ballRb->SetAngularSpeed(0.f);

	Utils::SetLocalPosToWorld(sUserPlatform, InitialUserPlatformPosition());
	if (auto u = sUserPlatform->FindBehaviour<UserPlatformControllerBehaviour>()) {
		u->ResyncSpawnFromNode();
	}

	Utils::SetLocalPosToWorld(sAiPlatform, InitialAiPlatformPosition());
	if (auto ai = sAiPlatform->FindBehaviour<AiPlatformControllerBehaviour>()) {
		ai->ResyncSpawnFromNode();
		ai->ClearPendingObservations();
	}
}
