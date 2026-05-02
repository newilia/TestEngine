#include "PongEnvironment.h"

#include "AiPlatformControllerBehaviour.h"
#include "Engine/App/FunctionInputHandler.h"
#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/CollisionBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/OverlappingBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
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
	auto& engine = Engine::MainContext::GetInstance();
	auto videoMode = sf::VideoMode::getFullscreenModes()[0];
	engine.CreateMainWindow(videoMode, "Pong", sf::Style::None);
	engine.GetMainWindow()->setMouseCursorVisible(false);
	engine.GetPhysicsProcessor()->SetGravity({0, 1000});
	engine.SetScene(BuildScene());
	ConfigureGlobalInput();
}

void PongEnvironment::AddBall(Scene* scene) {
	constexpr float radius = 35;
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = InitialBallVelocity();
	const sf::Vector2f pos = InitialBallPosition();

	auto ball = make_shared<PongBall>(CreateShapeBodyNode<sf::CircleShape>());
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

	auto rigidBody = ball->GetNode()->RequireBehaviour<RigidBodyBehaviour>();
	rigidBody->_mass = 3.14f * radius * radius;
	rigidBody->_restitution = bodiesRestitution;
	rigidBody->_velocity = vel;

	{
		auto ballCollision = ball->GetNode()->RequireBehaviour<CollisionBehaviour>();
		ballCollision->_collisionGroups.set(0, true);
		ballCollision->_collisionGroups.set(1, true);
	}
	ball->GetNode()->RequireBehaviour<OverlappingBehaviour>()->_overlappingGroups.set(0, true);
	scene->AddChild(ball->GetNode());

	sBall = ball;
}

shared_ptr<SceneNode> PongEnvironment::CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
                                                             sf::Color color) const {
	auto node = CreateShapeBodyNode<sf::ConvexShape>();

	node->SetName("Platform");

	constexpr int pointCount = 42;
	constexpr float curvature = 0.9f;

	auto shape = dynamic_cast<sf::ConvexShape*>(node->FindShapeCollider()->GetBaseShape());
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

	node->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(1, true);

	auto rigidBody = node->RequireBehaviour<RigidBodyBehaviour>();
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
		auto wall = CreateShapeBodyNode<sf::RectangleShape>();
		wall->SetName(wallNames[i]);
		auto* rect = dynamic_cast<sf::RectangleShape*>(wall->FindShapeCollider()->GetBaseShape());
		rect->setSize(wallSizes[i]);
		rect->setOrigin(Utils::FindCenterOfMass(rect));
		rect->setPosition(wallPositions[i]);
		auto wrb = wall->RequireBehaviour<RigidBodyBehaviour>();
		wrb->SetImmovable();
		wrb->_restitution = bodiesRestitution;
		if (i < 2) {
			rect->setFillColor(sf::Color(200, 200, 200, 50));
			wall->RequireBehaviour<OverlappingBehaviour>()->_overlappingGroups.set(0, true);
			if (i == 0) {
				auto loseCallback = createDelegate<const IntersectionDetails&>([this](const IntersectionDetails&) {
					OnLose();
				});
				static_cast<void>(wall->FindBehaviour<OverlappingBehaviour>()->_overlappingCallbacks.Connect(
				    std::move(loseCallback)));
			}
			else {
				auto winCallback = createDelegate<const IntersectionDetails&>([this](const IntersectionDetails&) {
					OnWin();
				});
				static_cast<void>(
				    wall->FindBehaviour<OverlappingBehaviour>()->_overlappingCallbacks.Connect(std::move(winCallback)));
			}
		}
		else {
			rect->setFillColor(sf::Color(200, 200, 200, 255));
			wall->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(0, true);
		}
		scene->AddChild(std::move(wall));
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
	_scoreText->setString(fmt::format("{}:{}", _userScore, _aiScore));
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

void PongEnvironment::ConfigureGlobalInput() {
	auto mainContext = &Engine::MainContext::GetInstance();

	std::make_shared<FunctionInputHandler>([this, mainContext](const sf::Event& event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::R) {
				mainContext->SetScene(BuildScene());
			}
			else if (key->code == sf::Keyboard::Key::D) {
				mainContext->SetDebugDrawEnabled(!mainContext->IsDebugDrawEnabled());
			}
			else if (key->code == sf::Keyboard::Key::Escape) {
				std::exit(EXIT_SUCCESS);
			}
		}
	})->Register();
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
	auto ballRb = sBall->GetNode()->RequireBehaviour<RigidBodyBehaviour>();
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
