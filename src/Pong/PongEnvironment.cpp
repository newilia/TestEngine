#include "PongEnvironment.h"

#include "AiPlatformController.h"
#include "Engine/Physics/PhysicsDebugComponent.h"
#include "Engine/FpsNode.h"
#include "PongBall.h"
#include "Engine/EngineInterface.h"
#include "Engine/Scene.h"
#include "Engine/Physics/ShapeBody.h"
#include "Engine/UserInput.h"
#include "fmt/format.h"
#include "Engine/Physics/CollisionComponent.h"
#include "PongPlatform.h"
#include "Engine/Physics/PhysicsHandler.h"
#include "Engine/Physics/OverlappingComponent.h"
#include "UserPlatformContoller.h"

constexpr float bodiesRestitution = 1;
constexpr float wallActualWidth = 200;
constexpr float wallVisibleWidth = 10;
static weak_ptr<PongPlatform> sUserPlatform;
static weak_ptr<PongBall> sBall;

static sf::Vector2f getScreenSize() {
	return sf::Vector2f(EI()->getMainWindow()->getSize());
}

void PongEnvironment::setup() {
	auto ei = EI();
	auto videoMode = sf::VideoMode::getFullscreenModes()[0];
	//sf::VideoMode videoMode(800, 600);
	ei->createMainWindow(videoMode, "Pong", sf::Style::None);
	ei->getMainWindow()->setMouseCursorVisible(false);
	ei->getPhysicsHandler()->setSubstepCount(2);
	ei->getPhysicsHandler()->setGravity({ 0, 1000 });
	ei->setDebugEnabled(false);
	//ei->getMainWindow()->setFramerateLimit(40.f);
	ei->setScene(buildScene());
	configureGlobalInput();
}

void PongEnvironment::addBall(Scene* scene) {
	constexpr float radius = 35;
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = { 0, 500 };
	const sf::Vector2f pos = getScreenSize() * 0.5f;

	auto ball = make_shared<PongBall>();
	ball->setMaxSpeed(400.f);
	ball->setSpeedDampingFactor(speedDampingFactor);
	auto shape = ball->getShape();
	ball->setName("Ball");

	shape->setRadius(radius);
	auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
	shape->setPointCount(pointsCount);
	auto outlineColor = color;
	outlineColor.a = 255;

	shape->setFillColor(color);
	shape->setOutlineColor(outlineColor);
	shape->setOutlineThickness(1);
	shape->setPosition(pos);

	auto pc = ball->getPhysicalComponent();
	pc->mMass = 3.14f * radius * radius;
	pc->mRestitution = bodiesRestitution;
	pc->mVelocity = vel;

	ball->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
	ball->requireComponent<CollisionComponent>()->mCollisionGroups.set(1, true);
	ball->requireComponent<OverlappingComponent>()->mOverlappingGroups.set(0, true);
	ball->init();
	scene->addChild(ball);

	sBall = ball;
}

shared_ptr<PongPlatform> PongEnvironment::createDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg, sf::Color color) const {
	auto platform = make_shared<PongPlatform>();
	platform->requireComponent<PhysicsDebugComponent>();
	platform->setName("Platform");
	platform->setShapeDimensions(size, 0.9f, rotationDeg);
	//todo platform->setMoveArea(...);
	platform->getShape()->setPosition(pos);
	platform->getShape()->setFillColor(color);

	platform->requireComponent<CollisionComponent>()->mCollisionGroups.set(1, true);

	auto pc = platform->getPhysicalComponent();
	pc->setImmovable();
	pc->mRestitution = bodiesRestitution;
	
	return platform;
}

void PongEnvironment::addWalls(Scene* scene) {
	auto screenSize = getScreenSize();
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;

	std::string wallNames[] = { "bottom", "top", "left", "right" };
	sf::Vector2f wallSizes[] = {
		{ screenSize.x, wallActualWidth },
		{ screenSize.x, wallActualWidth },
		{ wallActualWidth, screenSize.y },
		{ wallActualWidth, screenSize.y }
	};
	sf::Vector2f wallPositions[] = {
		{ screenSize.x / 2, screenSize.y + wallOffset },
		{ screenSize.x / 2, -wallOffset },
		{ -wallOffset, screenSize.y / 2 },
		{ screenSize.x + wallOffset, screenSize.y / 2 }
	};
	for (int i = 0; i < 4; ++i) {
		auto wall = make_shared<RectangleBody>();
		wall->setName(wallNames[i]);
		wall->getShape()->setPosition(wallPositions[i]);
		wall->getShape()->setSize(wallSizes[i]);
		wall->getPhysicalComponent()->setImmovable();
		wall->getPhysicalComponent()->mRestitution = bodiesRestitution;
		if (i < 2) {
			wall->getShape()->setFillColor(sf::Color(200, 200, 200, 50));
			wall->requireComponent<OverlappingComponent>()->mOverlappingGroups.set(0, true);
			auto loseCallback = createDelegate<const IntersectionDetails&>([this, calledOnce = false](const IntersectionDetails&) mutable {
				if (!calledOnce) {
					onLose();
					calledOnce = true;
				}
			});
			wall->findComponent<OverlappingComponent>()->mOverlappingCallbacks.push_back(std::move(loseCallback));
		}
		else {
			wall->getShape()->setFillColor(sf::Color(200, 200, 200, 255));
			wall->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		}
		wall->init();
		scene->addChild(wall);
	}
}

void PongEnvironment::addUserPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const auto screenSize = getScreenSize();
	const sf::Vector2f pos(screenSize.x * 0.5f, screenSize.y - wallVisibleWidth - 100);
	const sf::Color color(220, 220, 20);

	auto platform = createDefaultPlatform(size, pos, 180.f, color);
	platform->setName("User_platform");

	constexpr float verticalMoveFactor = 0.95f;
	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = { 15000.f, 1000.f };

	auto controller = make_shared<UserPlatformController>(platform.get());
	platform->setController(controller);
	controller->setVerticalFreedomFactor(verticalMoveFactor);
	controller->setVelLimit(maxSpeed);
	controller->setVelocityFactor(velFactor);

	platform->init();
	scene->addChild(platform);

	EI()->getUserInput()->attachEventHandler(createDelegate<PongPlatform, sf::Event>(platform, [platform = std::weak_ptr(platform)](sf::Event event) {
		if (auto controller = dynamic_pointer_cast<UserPlatformController>(platform.lock()->getController())) {
			controller->handleEvent(event);
		}
	}));
	sUserPlatform = platform;
}

void PongEnvironment::addAiPlatform(Scene* scene) {
	const sf::Vector2f size(500.f, 70.f);
	const auto screenSize = getScreenSize();
	const sf::Vector2f pos(screenSize.x * 0.5f, wallVisibleWidth + 100);
	const sf::Color color(220, 220, 20);

	auto platform = createDefaultPlatform(size, pos, 0.f, color);;
	platform->setName("AI_platform");

	auto controller = make_shared<AiPlatformController>(platform.get());
	controller->beginObserve(sUserPlatform, sBall);
	platform->setController(controller);

	constexpr float verticalMoveFactor = 0.99f;
	constexpr float velFactor = 50.f;
	const sf::Vector2f maxSpeed = { 3000.f, 1000.f };
	
	controller->setVerticalFreedomFactor(verticalMoveFactor);
	controller->setVelLimit(maxSpeed);
	controller->setVelocityFactor(velFactor);
	controller->setObservePeriod(sf::milliseconds(10));
	controller->setReactionDelay(sf::milliseconds(100));

	platform->init();
	scene->addChild(platform);
}

std::shared_ptr<Scene> PongEnvironment::buildScene() {
	auto scene = make_shared<Scene>();
	
	addWalls(scene.get());
	addBall(scene.get());
	addUserPlatform(scene.get());
	addAiPlatform(scene.get());
	scene->addChild(make_shared<FpsNode>());
	return scene;
}

void PongEnvironment::configureGlobalInput() {
	auto ei = EI();
	auto userInput = ei->getUserInput();
	auto scene = ei->getScene();
	
	userInput->attachEventHandler(createDelegate<sf::Event>([this, ei](sf::Event event) {
		if (event.type == sf::Event::EventType::KeyPressed) {
			if (event.key.code == sf::Keyboard::R) {
				ei->setScene(buildScene());
			}
			else if (event.key.code == sf::Keyboard::D) {
				ei->setDebugEnabled(!ei->isDebugEnabled());
			}
			else if (event.key.code == sf::Keyboard::Escape) {
				std::exit(EXIT_SUCCESS);
			}
		}
	}));
}

void PongEnvironment::onLose() {
	EI()->setScene(buildScene());
}
