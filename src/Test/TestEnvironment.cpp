#include "TestEnvironment.h"

#include "Engine/FpsNode.h"
#include "Engine/Physics/AbstractShapeBody.h"
#include "Engine/EngineInterface.h"
#include "Engine/Scene.h"
#include "Engine/Physics/ShapeBody.h"
#include "Engine/UserInput.h"
#include "fmt/format.h"
#include "Engine/Physics/BodyPullHandler.h"
#include "Engine/Physics/CollisionComponent.h"
#include "Engine/Physics/PhysicsHandler.h"

void TestEnvironment::setup() {
	auto ei = EI();
	ei->createMainWindow(sf::VideoMode(800u, 600u), "Test scene", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	ei->getPhysicsHandler()->setSubstepCount(2);
	ei->getPhysicsHandler()->setGravity({ 0, 1000 });
	ei->setScene(buildScene());
	configureInput();
}

std::shared_ptr<Scene> TestEnvironment::buildScene() {
	auto scene = make_shared<Scene>();
	sf::Vector2f screenSize(EI()->getMainWindow()->getSize());
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	float bodiesRestitution = 0.9f;

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
		body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
		body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
		body->getShape()->setOutlineThickness(1.f);
		body->getPhysicalComponent()->setImmovable();
		body->getPhysicalComponent()->mRestitution = bodiesRestitution;
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		body->init();
		scene->addChild(body);
	}

	for (int i = 0; i < 300; ++i) {
		auto body = make_shared<CircleBody>();
		body->setName(fmt::format("circle_{}", i));

		float radius = 5.f * (1 + i % 3);
		body->getShape()->setRadius(radius);
		constexpr float pointsCountConstant = 3.f;
		auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
		body->getShape()->setPointCount(pointsCount);
		
		sf::Color color(40, 170, 255, 200);
		auto outlineColor = color;
		outlineColor.a = 255;

		body->getShape()->setFillColor(color);
		body->getShape()->setOutlineColor(outlineColor);
		body->getShape()->setOutlineThickness(1);
		auto minX = static_cast<int>(wallVisibleWidth + radius);
		auto maxX = static_cast<int>(screenSize.x - wallVisibleWidth - radius);
		auto minY = static_cast<int>(wallVisibleWidth + radius);
		auto maxY = static_cast<int>(screenSize.y - wallVisibleWidth - radius);
		auto x = static_cast<float>(minX + rand() % (maxX - minX));
		auto y = static_cast<float>(minY + rand() % (maxY - minY));
		body->getShape()->setPosition(x, y);
		body->getPhysicalComponent()->mMass = 3.14f * radius * radius;
		body->getPhysicalComponent()->mRestitution = bodiesRestitution;
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(0, true);
		body->init();
		scene->addChild(body);
	}
	scene->addChild(make_shared<FpsNode>());
	return scene;
}

void TestEnvironment::configureInput() {
	auto ei = EI();
	auto userInput = ei->getUserInput();
	auto scene = ei->getScene();

	userInput->attachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::R) {
			ei->setScene(buildScene());
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		switch (event.type) {
		case sf::Event::MouseButtonPressed: {
			sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
			if (event.mouseButton.button == sf::Mouse::Button::Left) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::FORCE);
			}
			else if (event.mouseButton.button == sf::Mouse::Button::Right) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::POSITION);
			}
			else if (event.mouseButton.button == sf::Mouse::Button::Middle) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::VELOCITY);
			}
			break;
		}

		case sf::Event::MouseButtonReleased:
			ei->getBodyPullHandler()->stopPull();
			break;

		case sf::Event::MouseMoved:
			ei->getBodyPullHandler()->setPullDestination(sf::Vector2f(static_cast<float>(event.mouseMove.x), 
				static_cast<float>(event.mouseMove.y)));
			break;
		default:
			break;
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code) {
			case sf::Keyboard::Equal:
				ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() * 2);
				break;
			case sf::Keyboard::Hyphen:
				ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() * 0.5f);
				break;
			case sf::Keyboard::Num0:
				ei->setSimPaused(!ei->isSimPaused());
				break;
			default: break;
			}
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([](sf::Event event) {
		if (event.type == sf::Event::Closed) {
			std::exit(EXIT_SUCCESS);
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G) {
			ei->getPhysicsHandler()->setGravityEnabled(!ei->getPhysicsHandler()->isGravityEnabled());
		}
	}));

	userInput->attachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) {
			ei->setDebugEnabled(!ei->isDebugEnabled());
		}
	}));
}
