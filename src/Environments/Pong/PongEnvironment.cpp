#include "PongEnvironment.h"

#include "AiPlatformControllerBehaviour.h"
#include "Engine/App/FontManager.h"
#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/TextVisual.h"
#include "PongBall.h"
#include "PongPlayfield.h"
#include "UserPlatformControllerBehaviour.h"

#include <SFML/Graphics/Text.hpp>

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

PongEnvironment::~PongEnvironment() = default;

void PongEnvironment::Setup() {
	auto& mainContext = Engine::MainContext::GetInstance();
	auto videoMode = sf::VideoMode::getFullscreenModes()[0];
	mainContext.CreateMainWindow(videoMode, "Pong", sf::Style::None);
	mainContext.GetMainWindow()->setMouseCursorVisible(false);
	mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
	mainContext.SetScene(BuildScene());
}

void PongEnvironment::OnEvent(const sf::Event& event) {
	if (auto e = event.getIf<sf::Event::KeyPressed>()) {
		auto& mainContext = Engine::MainContext::GetInstance();
		if (e->code == sf::Keyboard::Key::R) {
			mainContext.SetScene(BuildScene());
		}
		else if (e->code == sf::Keyboard::Key::D) {
			mainContext.SetDebugDrawEnabled(!mainContext.IsDebugDrawEnabled());
		}
		else if (e->code == sf::Keyboard::Key::Escape) {
			std::exit(EXIT_SUCCESS);
		}
	}
}

void PongEnvironment::AddBall(Scene* scene) {
	constexpr float radius = 35;
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = InitialBallVelocity();
	const sf::Vector2f pos = InitialBallPosition();

	auto ball = make_shared<PongBall>(CreatePhysicsBodyNode<sf::CircleShape>());
	ball->SetupBehaviours();
	ball->SetMaxSpeed(400.f);
	ball->SetSpeedDampingFactor(speedDampingFactor);
	auto shape = ball->GetShape();
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
	shape->setPosition(pos);

	auto rigidBody = ball->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
	rigidBody->_mass = 3.14f * radius * radius;
	rigidBody->_restitution = bodiesRestitution;
	rigidBody->_velocity = vel;
	rigidBody->_collisionGroups.set(0, true);
	rigidBody->_collisionGroups.set(1, true);
	rigidBody->_overlappingGroups.set(0, true);
	scene->AddChild(ball->GetNode());

	sBall = ball;
}

shared_ptr<SceneNode> PongEnvironment::CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
                                                             sf::Color color) const {
	auto node = CreatePhysicsBodyNode<sf::ConvexShape>();
	node->SetName("Platform");

	constexpr int pointCount = 42;
	constexpr float curvature = 0.9f;

	auto shape = dynamic_cast<sf::ConvexShape*>(node->FindPhysicsBody()->GetShape());
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
	shape->setPosition(pos);
	shape->setFillColor(color);

	auto rigidBody = node->RequireBehaviour<PhysicsBodyBehaviour>();
	rigidBody->_collisionGroups.set(1, true);
	rigidBody->SetImmovable();
	rigidBody->_restitution = bodiesRestitution;

	return node;
}

void PongEnvironment::AddWalls(Scene* scene) {
	const auto field = GetPongPlayfieldRect();
	const sf::Vector2f o = field.position;
	const sf::Vector2f sz = field.size;

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {
	    {sz.x, wallActualWidth}, {sz.x, wallActualWidth}, {wallActualWidth, sz.y}, {wallActualWidth, sz.y}};

	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	sf::Vector2f wallPositions[] = {{o.x + sz.x / 2, o.y + sz.y + wallOffset},
	                                {o.x + sz.x / 2, o.y - wallOffset},
	                                {o.x - wallOffset, o.y + sz.y / 2},
	                                {o.x + sz.x + wallOffset, o.y + sz.y / 2}};
	for (int i = 0; i < 4; ++i) {
		auto wallNode = CreatePhysicsBodyNode<sf::RectangleShape>();
		wallNode->SetName(wallNames[i]);

		auto* rectShape = dynamic_cast<sf::RectangleShape*>(wallNode->FindPhysicsBody()->GetShape());
		rectShape->setSize(wallSizes[i]);
		rectShape->setOrigin(Utils::FindCenterOfMass(rectShape));
		rectShape->setPosition(wallPositions[i]);

		auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->SetImmovable();
		bodyBeh->_restitution = bodiesRestitution;

		if (i < 2) {
			rectShape->setFillColor(sf::Color(200, 200, 200, 50));
			bodyBeh->_overlappingGroups.set(0, true);

			if (i == 0) {
				[[maybe_unused]] auto connection =
				    bodyBeh->_overlappingCallbacks.Connect([this](const IntersectionDetails&) {
					    OnLose();
				    });
			}
			else {
				[[maybe_unused]] auto connection =
				    bodyBeh->_overlappingCallbacks.Connect([this](const IntersectionDetails&) {
					    OnWin();
				    });
			}
		}
		else {
			rectShape->setFillColor(sf::Color(200, 200, 200, 255));
			bodyBeh->_collisionGroups.set(0, true);
		}
		scene->AddChild(std::move(wallNode));
	}
}

void PongEnvironment::AddUserPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const sf::Vector2f pos = InitialUserPlatformPosition();
	const sf::Color color(220, 220, 20);

	auto platformNode = CreateDefaultPlatform(size, pos, 180.f, color);
	platformNode->SetName("User_platform");

	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = {15000.f, 1000.f};

	auto userBehaviour = std::make_shared<UserPlatformControllerBehaviour>();
	platformNode->AddBehaviour(userBehaviour);
	userBehaviour->_velLimit = maxSpeed;
	userBehaviour->_speedFactor = velFactor;

	scene->AddChild(platformNode);

	sUserPlatform = platformNode;
}

void PongEnvironment::AddAiPlatform(Scene* scene) {
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

	aiBehaviour->_velLimit = maxSpeed;
	aiBehaviour->_speedFactor = velFactor;
	aiBehaviour->SetObservePeriod(sf::milliseconds(10));
	aiBehaviour->SetReactionDelay(sf::milliseconds(100));

	scene->AddChild(platformNode);

	sAiPlatform = platformNode;
}

void PongEnvironment::AddScoreboard(Scene* scene) {
	auto* font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont();
	if (!font) {
		return;
	}

	_userScore = 0;
	_aiScore = 0;

	auto node = std::make_shared<SceneNode>();
	auto sorting = std::make_shared<RelativeSortingStrategy>();
	sorting->SetPriority(-1);
	node->SetSortingStrategy(sorting);
	node->SetName("Score");

	_scoreText = std::make_shared<sf::Text>(*font, "0:0", static_cast<unsigned>(kPongScoreFontSize));
	_scoreText->setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(0.3f * 255.f)));

	const sf::Vector2f center = GetPongPlayfieldRect().getCenter();
	auto lb = _scoreText->getLocalBounds();
	_scoreText->setOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
	_scoreText->setPosition(center);

	node->SetVisual(std::make_shared<TextVisual>(_scoreText));
	scene->AddChild(std::move(node));
}

void PongEnvironment::UpdateScoreText() {
	if (!_scoreText) {
		return;
	}
	auto text = fmt::format("{}:{}", _userScore, _aiScore);
	_scoreText->setString(text);
	auto lb = _scoreText->getLocalBounds();
	_scoreText->setOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
	_scoreText->setPosition(GetPongPlayfieldRect().getCenter());
}

std::shared_ptr<Scene> PongEnvironment::BuildScene() {
	auto scene = make_shared<Scene>();

	AddWalls(scene.get());
	AddBall(scene.get());
	AddUserPlatform(scene.get());
	AddAiPlatform(scene.get());
	AddScoreboard(scene.get());
	return scene;
}

void PongEnvironment::OnLose() {
	++_aiScore;
	UpdateScoreText();
	ResetRound();
}

void PongEnvironment::OnWin() {
	++_userScore;
	UpdateScoreText();
	ResetRound();
}

void PongEnvironment::ResetRound() {
	if (!sBall || !sUserPlatform || !sAiPlatform) {
		return;
	}

	sBall->GetNode()->SetPosGlobal(InitialBallPosition());
	auto ballRb = sBall->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
	ballRb->_velocity = InitialBallVelocity();
	ballRb->_angularSpeed = 0.f;

	sUserPlatform->SetPosGlobal(InitialUserPlatformPosition());
	if (auto u = sUserPlatform->FindBehaviour<UserPlatformControllerBehaviour>()) {
		u->ResyncSpawnFromNode();
	}

	sAiPlatform->SetPosGlobal(InitialAiPlatformPosition());
	if (auto ai = sAiPlatform->FindBehaviour<AiPlatformControllerBehaviour>()) {
		ai->ResyncSpawnFromNode();
		ai->ClearPendingObservations();
	}
}
