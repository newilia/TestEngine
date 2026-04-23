#include "TestEnvironment.h"

#include "Engine/EngineInterface.h"
#include "Engine/FpsNode.h"
#include "Engine/Physics/AbstractShapeBody.h"
#include "Engine/Physics/BodyPullHandler.h"
#include "Engine/Physics/CollisionComponent.h"
#include "Engine/Physics/PhysicsHandler.h"
#include "Engine/Physics/ShapeBody.h"
#include "Engine/Scene.h"
#include "Engine/UserInput.h"
#include "fmt/format.h"

void TestEnvironment::setup() {
	EngineContext& engine = EngineContext::instance();
	engine.createMainWindow(sf::VideoMode({800u, 600u}), "Test scene",
	                        sf::Style::Titlebar | sf::Style::Close |
	                            sf::Style::Resize);
	engine.getPhysicsHandler()->setSubstepCount(2);
	engine.getPhysicsHandler()->setGravity({0, 1000});
	engine.setScene(buildScene());
	configureInput();
}

std::shared_ptr<Scene> TestEnvironment::buildScene() {
	auto scene = make_shared<Scene>();
	sf::Vector2f screenSize(EngineContext::instance().getMainWindow()->getSize());
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	float bodiesRestitution = 0.9f;

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {{screenSize.x, wallActualWidth},
	                            {screenSize.x, wallActualWidth},
	                            {wallActualWidth, screenSize.y},
	                            {wallActualWidth, screenSize.y}};
	sf::Vector2f wallPositions[] = {
	    {screenSize.x / 2, screenSize.y + wallOffset},
	    {screenSize.x / 2, -wallOffset},
	    {-wallOffset, screenSize.y / 2},
	    {screenSize.x + wallOffset, screenSize.y / 2}};
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
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(
		    0, true);
		body->init();
		scene->addChild(body);
	}

	for (int i = 0; i < 300; ++i) {
		auto body = make_shared<CircleBody>();
		body->setName(fmt::format("circle_{}", i));

		float radius = 5.f * (1 + i % 3);
		body->getShape()->setRadius(radius);
		constexpr float pointsCountConstant = 3.f;
		auto pointsCount =
		    static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
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
		body->getShape()->setPosition(sf::Vector2f{x, y});
		body->getPhysicalComponent()->mMass = 3.14f * radius * radius;
		body->getPhysicalComponent()->mRestitution = bodiesRestitution;
		body->requireComponent<CollisionComponent>()->mCollisionGroups.set(
		    0, true);
		body->init();
		scene->addChild(body);
	}
	scene->addChild(make_shared<FpsNode>());
	return scene;
}

void TestEnvironment::configureInput() {
	auto ei = &EngineContext::instance();
	auto userInput = ei->getUserInput();
	auto scene = ei->getScene();

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([ei](sf::Event event) {
		    if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
			    if (key->code == sf::Keyboard::Key::R) {
				    ei->setScene(buildScene());
			    }
		    }
	    }));

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([ei](sf::Event event) {
		    if (const auto *pressed =
		            event.getIf<sf::Event::MouseButtonPressed>()) {
			    sf::Vector2f mousePos(static_cast<float>(pressed->position.x),
			                          static_cast<float>(pressed->position.y));
			    if (pressed->button == sf::Mouse::Button::Left) {
				    ei->getBodyPullHandler()->startPull(
				        mousePos, UserPullComponent::PullMode::FORCE);
			    } else if (pressed->button == sf::Mouse::Button::Right) {
				    ei->getBodyPullHandler()->startPull(
				        mousePos, UserPullComponent::PullMode::POSITION);
			    } else if (pressed->button == sf::Mouse::Button::Middle) {
				    ei->getBodyPullHandler()->startPull(
				        mousePos, UserPullComponent::PullMode::VELOCITY);
			    }
		    } else if (event.is<sf::Event::MouseButtonReleased>()) {
			    ei->getBodyPullHandler()->stopPull();
		    } else if (const auto *moved =
		                   event.getIf<sf::Event::MouseMoved>()) {
			    ei->getBodyPullHandler()->setPullDestination(
			        sf::Vector2f(static_cast<float>(moved->position.x),
			                     static_cast<float>(moved->position.y)));
		    }
	    }));

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([ei](sf::Event event) {
		    if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
			    switch (key->code) {
			    case sf::Keyboard::Key::Equal:
				    ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() * 2);
				    break;
			    case sf::Keyboard::Key::Hyphen:
				    ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() *
				                              0.5f);
				    break;
			    case sf::Keyboard::Key::Num0:
				    ei->setSimPaused(!ei->isSimPaused());
				    break;
			    default:
				    break;
			    }
		    }
	    }));

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([](sf::Event event) {
		    if (event.is<sf::Event::Closed>()) {
			    std::exit(EXIT_SUCCESS);
		    }
	    }));

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([ei](sf::Event event) {
		    if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
			    if (key->code == sf::Keyboard::Key::G) {
				    ei->getPhysicsHandler()->setGravityEnabled(
				        !ei->getPhysicsHandler()->isGravityEnabled());
			    }
		    }
	    }));

	userInput->attachEventHandler(
	    createDelegate<sf::Event>([ei](sf::Event event) {
		    if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
			    if (key->code == sf::Keyboard::Key::D) {
				    ei->setDebugEnabled(!ei->isDebugEnabled());
			    }
		    }
	    }));
}
