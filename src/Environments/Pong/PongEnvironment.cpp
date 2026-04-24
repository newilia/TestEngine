#include "PongEnvironment.h"

#include "AiPlatformController.h"
#include "Engine/App/EngineInterface.h"
#include "Engine/Behaviour/FpsCounterBehaviour.h"
#include "Engine/Behaviour/Physics/CollisionBehaviour.h"
#include "Engine/Behaviour/Physics/OverlappingBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsHandler.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/App/Utils.h"
#include "Engine/App/UserInput.h"
#include "PongBall.h"
#include "PongPlatform.h"
#include "UserPlatformController.h"
#include "fmt/format.h"

#include <utility>

using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;

constexpr float bodiesRestitution = 1;
constexpr float wallActualWidth = 200;
constexpr float wallVisibleWidth = 10;
static weak_ptr<PongPlatform> sUserPlatform;
static weak_ptr<PongBall> sBall;

namespace {

sf::Vector2f GetScreenSize() {
	return sf::Vector2f(EngineContext::Instance().GetMainWindow()->getSize());
}

} // namespace

void PongEnvironment::Setup() {
	EngineContext& engine = EngineContext::Instance();
	auto videoMode = sf::VideoMode::getFullscreenModes()[0];
	// sf::VideoMode videoMode(800, 600);
	engine.CreateMainWindow(videoMode, "Pong", sf::Style::None);
	engine.GetMainWindow()->setMouseCursorVisible(false);
	engine.GetPhysicsHandler()->SetSubstepCount(2);
	engine.GetPhysicsHandler()->SetGravity({0, 1000});
	engine.SetDebugEnabled(false);
	// engine.GetMainWindow()->setFramerateLimit(40.f);
	engine.SetScene(BuildScene());
	ConfigureGlobalInput();
}

void PongEnvironment::AddBall(Scene* scene) {
	constexpr float radius = 35;
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = {0, 500};
	const sf::Vector2f pos = GetScreenSize() * 0.5f;

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
	shape->setOrigin(utils::FindCenterOfMass(shape));
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

shared_ptr<PongPlatform> PongEnvironment::CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
                                                                sf::Color color) const {
	auto platform = make_shared<PongPlatform>(CreateShapeBodyNode<sf::ConvexShape>());
	platform->RegisterTickBehaviour();
	platform->GetNode()->RequireBehaviour<PhysicsDebugBehaviour>();
	platform->SetName("Platform");
	platform->SetShapeDimensions(size, 0.9f, rotationDeg);
	// todo platform->setMoveArea(...);
	if (auto* sh = platform->GetShape()) {
		sh->setOrigin(utils::FindCenterOfMass(sh));
		sh->setPosition(pos);
		sh->setFillColor(color);
	}

	platform->GetNode()->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(1, true);

	auto rigidBody = platform->GetNode()->RequireBehaviour<RigidBodyBehaviour>();
	rigidBody->SetImmovable();
	rigidBody->_restitution = bodiesRestitution;

	return platform;
}

void PongEnvironment::AddWalls(Scene* scene) {
	auto screenSize = GetScreenSize();
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {{screenSize.x, wallActualWidth},
	                            {screenSize.x, wallActualWidth},
	                            {wallActualWidth, screenSize.y},
	                            {wallActualWidth, screenSize.y}};
	sf::Vector2f wallPositions[] = {{screenSize.x / 2, screenSize.y + wallOffset},
	                                {screenSize.x / 2, -wallOffset},
	                                {-wallOffset, screenSize.y / 2},
	                                {screenSize.x + wallOffset, screenSize.y / 2}};
	for (int i = 0; i < 4; ++i) {
		auto wall = CreateShapeBodyNode<sf::RectangleShape>();
		wall->SetName(wallNames[i]);
		auto* rect = dynamic_cast<sf::RectangleShape*>(wall->FindShapeCollider()->GetBaseShape());
		rect->setSize(wallSizes[i]);
		rect->setOrigin(utils::FindCenterOfMass(rect));
		rect->setPosition(wallPositions[i]);
		auto wrb = wall->RequireBehaviour<RigidBodyBehaviour>();
		wrb->SetImmovable();
		wrb->_restitution = bodiesRestitution;
		if (i < 2) {
			rect->setFillColor(sf::Color(200, 200, 200, 50));
			wall->RequireBehaviour<OverlappingBehaviour>()->_overlappingGroups.set(0, true);
			auto loseCallback = createDelegate<const IntersectionDetails&>(
			    [this, calledOnce = false](const IntersectionDetails&) mutable {
				    if (!calledOnce) {
					    OnLose();
					    calledOnce = true;
				    }
			    });
			wall->FindBehaviour<OverlappingBehaviour>()->_overlappingCallbacks.push_back(std::move(loseCallback));
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
	const auto screenSize = GetScreenSize();
	const sf::Vector2f pos(screenSize.x * 0.5f, screenSize.y - wallVisibleWidth - 100);
	const sf::Color color(220, 220, 20);

	auto platform = CreateDefaultPlatform(size, pos, 180.f, color);
	platform->SetName("User_platform");

	constexpr float verticalMoveFactor = 0.95f;
	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = {15000.f, 1000.f};

	auto controller = make_shared<UserPlatformController>(platform.get());
	platform->SetController(controller);
	controller->SetVerticalFreedomFactor(verticalMoveFactor);
	controller->SetVelLimit(maxSpeed);
	controller->SetVelocityFactor(velFactor);

	platform->Init();
	scene->AddChild(platform->GetNode());

	EngineContext::Instance().GetUserInput()->AttachEventHandler(
	    createDelegate<PongPlatform, sf::Event>(platform, [platform = std::weak_ptr(platform)](sf::Event event) {
		    if (auto c = dynamic_pointer_cast<UserPlatformController>(platform.lock()->GetController())) {
			    c->HandleEvent(event);
		    }
	    }));
	sUserPlatform = platform;
}

void PongEnvironment::AddAiPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const auto screenSize = GetScreenSize();
	const sf::Vector2f pos(screenSize.x * 0.5f, wallVisibleWidth + 100);
	const sf::Color color(220, 220, 20);

	auto platform = CreateDefaultPlatform(size, pos, 0.f, color);
	;
	platform->SetName("AI_platform");

	auto controller = make_shared<AiPlatformController>(platform.get());
	controller->BeginObserve(sUserPlatform, sBall);
	platform->SetController(controller);

	constexpr float verticalMoveFactor = 0.99f;
	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = {3000.f, 1000.f};

	controller->SetVerticalFreedomFactor(verticalMoveFactor);
	controller->SetVelLimit(maxSpeed);
	controller->SetVelocityFactor(velFactor);
	controller->SetObservePeriod(sf::milliseconds(10));
	controller->SetReactionDelay(sf::milliseconds(100));

	platform->Init();
	scene->AddChild(platform->GetNode());
}

std::shared_ptr<Scene> PongEnvironment::BuildScene() {
	auto scene = make_shared<Scene>();

	AddWalls(scene.get());
	AddBall(scene.get());
	AddUserPlatform(scene.get());
	AddAiPlatform(scene.get());
	scene->AddChild(CreateFpsCounterNode());
	return scene;
}

void PongEnvironment::ConfigureGlobalInput() {
	auto ei = &EngineContext::Instance();
	auto userInput = ei->GetUserInput();
	auto scene = ei->GetScene();

	userInput->AttachEventHandler(createDelegate<sf::Event>([this, ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::R) {
				ei->SetScene(BuildScene());
			}
			else if (key->code == sf::Keyboard::Key::D) {
				ei->SetDebugEnabled(!ei->IsDebugEnabled());
			}
			else if (key->code == sf::Keyboard::Key::Escape) {
				std::exit(EXIT_SUCCESS);
			}
		}
	}));
}

void PongEnvironment::OnLose() {
	EngineContext::Instance().SetScene(BuildScene());
}
