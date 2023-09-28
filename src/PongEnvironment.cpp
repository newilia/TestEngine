#include "PongEnvironment.h"

#include <iostream>

#include "AiPlatformController.h"
#include "PhysicsDebugComponent.h"
#include "FpsNode.h"
#include "PongBall.h"
#include "EngineInterface.h"
#include "Scene.h"
#include "ShapeBody.h"
#include "UserInput.h"
#include "fmt/format.h"
#include "BodyPullHandler.h"
#include "CollisionComponent.h"
#include "PongPlatform.h"
#include "Physics/PhysicsHandler.h"
#include "OverlappingComponent.h"
#include "UserPlatformContoller.h"

constexpr float bodiesRestitution = 1.f;
constexpr float wallActualWidth = 200;
constexpr float wallVisibleWidth = 10;
static weak_ptr<PongPlatform> sUserPlatform;
static weak_ptr<PongBall> sBall;

static sf::Vector2f getScreenSize() {
	return sf::Vector2f(EI()->getMainWindow()->getSize());
}

void PongEnvironment::setup() {
	auto ei = EI();
	//auto videoMode = sf::VideoMode::getFullscreenModes()[0];
	sf::VideoMode videoMode(800, 600);
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
	constexpr float radius = 50;
	constexpr float pointsCountConstant = 3.f;
	constexpr float speedDampingFactor = 0.1f;
	const sf::Color color(40, 170, 255, 200);
	const sf::Vector2f vel = { 0, 500 };

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
	shape->setPosition({ getScreenSize().x * 0.5f, radius + wallVisibleWidth});

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
	platform->init();
	platform->requireComponent<PhysicsDebugComponent>();
	platform->setName("Platform");
	platform->setShapeDimensions(size.x, size.y, rotationDeg);
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
		if (i == 0) {
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

	constexpr float verticalMoveFactor = 0.99f;
	constexpr float velFactor = 100.f;
	const sf::Vector2f maxSpeed = { 15000.f, 100000.f };

	auto controller = make_shared<UserPlatformController>(platform.get());
	//platform->setController(controller);
	controller->setVerticalFreedomFactor(verticalMoveFactor);
	controller->setVelLimit(maxSpeed);
	controller->setVelocityFactor(velFactor);

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

	auto controller = make_shared<AiPlatformController>(platform.get());
	controller->beginObserve(sUserPlatform, sBall);
	platform->setController(controller);

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
