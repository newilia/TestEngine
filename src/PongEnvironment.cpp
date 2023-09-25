#include "PongEnvironment.h"

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

void PongEnvironment::setup() {
	auto ei = EI();
	ei->createMainWindow(sf::VideoMode(800u, 600u), "Pong", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	ei->getPhysicsHandler()->setSubstepCount(2);
	ei->getPhysicsHandler()->setGravity({ 0, 1000 });
	ei->setDebugEnabled(false);
	//ei->getMainWindow()->setFramerateLimit(40.f);
	ei->setScene(buildScene());
	configureInput();
}

std::shared_ptr<Scene> PongEnvironment::buildScene() {
	auto scene = make_shared<Scene>();
	sf::Vector2f screenSize(EI()->getMainWindow()->getSize());
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 10;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	float bodiesRestitution = 1.f;

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
		auto body = make_shared<RectangleBody>();
		body->setName(wallNames[i]);
		body->getShape()->setPosition(wallPositions[i]);
		body->getShape()->setSize(wallSizes[i]);
		body->getPhysicalComponent()->setImmovable();
		body->getPhysicalComponent()->mRestitution = bodiesRestitution;
		if (i < 1) {
			body->getShape()->setFillColor(sf::Color(200, 200, 200, 50));
			body->requireComponent<OverlappingComponent>()->mOverlappingGroups.set(0, true);
		}
		else {
			body->getShape()->setFillColor(sf::Color(200, 200, 200, 255));
			body->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		}
		body->init();
		scene->addChild(body);
	}

	{
		auto body = make_shared<PongBall>();
		auto shape = body->getShape();
		auto pc = body->getPhysicalComponent();
		body->setName("Ball");

		float radius = 20;
		shape->setRadius(radius);
		constexpr float pointsCountConstant = 3.f;
		auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
		shape->setPointCount(pointsCount);

		sf::Color color(40, 170, 255, 200);
		auto outlineColor = color;
		outlineColor.a = 255;

		shape->setFillColor(color);
		shape->setOutlineColor(outlineColor);
		shape->setOutlineThickness(1);
		shape->setPosition({ screenSize.x * 0.5f, radius + wallVisibleWidth });

		pc->mMass = 3.14f * radius * radius;
		pc->mRestitution = bodiesRestitution;
		pc->mVelocity = { 0, 500 };
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		body->requireComponent<OverlappingComponent>()->mOverlappingGroups.set(0, true);
		body->init();
		scene->addChild(body);
	}

	{
		float width = 150.f, height = 30.f;
		auto body = make_shared<PongPlatform>();
		body->requireComponent<PhysicsDebugComponent>();
		body->setName("Platform");
		body->initShape(width, height, 180);
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		body->getShape()->setPosition(screenSize.x * 0.5f, screenSize.y - wallVisibleWidth - 100);
		body->getShape()->setFillColor(sf::Color(220, 220, 20));

		auto pc = body->getPhysicalComponent();
		pc->mMass = 0.5f;// width* height;
		pc->mRestitution = bodiesRestitution;

		auto& controller = body->getController();
		controller.setVerticalMoveFactor(0.99f);
		controller.setMaxSpeed({ 1500.f, 500.f });
		controller.setSpeedFactor(50.f);

		body->init();
		scene->addChild(body);

		EI()->getUserInput()->attachEventHandler(createDelegate<PongPlatform, sf::Event>(body, [platform = std::weak_ptr(body)](sf::Event event) {
			platform.lock()->getController().handleEvent(event);
		}));
	}
	scene->addChild(make_shared<FpsNode>());
	return scene;
}

void PongEnvironment::configureInput() {
	auto ei = EI();
	auto userInput = ei->getUserInput();
	auto scene = ei->getScene();
	
	userInput->attachEventHandler(createDelegate<sf::Event>([this, ei](sf::Event event) {
		if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::R) {
			ei->setScene(buildScene());
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([this, ei](sf::Event event) {
		if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::D) {
			ei->setDebugEnabled(!ei->isDebugEnabled());
		}
	}));
}
