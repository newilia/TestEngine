#include "TestEnvironment.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/App/FpsCounterBehaviour.h"
#include "Engine/Physics/BodyPullHandler.h"
#include "Engine/Physics/CollisionBehaviour.h"
#include "Engine/Physics/RigidBodyBehaviour.h"
#include "Engine/Physics/UserPullBehaviour.h"
#include "Engine/Physics/PhysicsHandler.h"
#include "Engine/Physics/ShapeColliderBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/App/UserInput.h"
#include "Engine/App/Utils.h"
#include "fmt/format.h"

#include <utility>

using std::make_shared;
using std::shared_ptr;

void TestEnvironment::Setup() {
	EngineContext& engine = EngineContext::Instance();
	engine.CreateMainWindow(sf::VideoMode({800u, 600u}), "Test scene",
	                        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	engine.GetPhysicsHandler()->SetSubstepCount(2);
	engine.GetPhysicsHandler()->SetGravity({0, 1000});
	engine.SetScene(BuildScene());
	ConfigureInput();
}

std::shared_ptr<Scene> TestEnvironment::BuildScene() {
	auto scene = make_shared<Scene>();
	sf::Vector2f screenSize(EngineContext::Instance().GetMainWindow()->getSize());
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	float bodiesRestitution = 0.9f;

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
		auto body = CreateShapeBodyNode<sf::RectangleShape>();
		body->SetName(wallNames[i]);
		auto* rect = dynamic_cast<sf::RectangleShape*>(body->FindShapeCollider()->GetBaseShape());
		rect->setSize(wallSizes[i]);
		rect->setOrigin(utils::FindCenterOfMass(rect));
		rect->setPosition(wallPositions[i]);
		rect->setFillColor(sf::Color(30, 255, 30, 50));
		rect->setOutlineColor(sf::Color(30, 255, 30, 120));
		rect->setOutlineThickness(1.f);
		auto wrb = body->RequireBehaviour<RigidBodyBehaviour>();
		wrb->SetImmovable();
		wrb->_restitution = bodiesRestitution;
		body->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(0, true);
		scene->AddChild(std::move(body));
	}

	for (int i = 0; i < 300; ++i) {
		auto body = CreateShapeBodyNode<sf::CircleShape>();
		body->SetName(fmt::format("circle_{}", i));

		float radius = 5.f * (1 + i % 3);
		auto* circle = dynamic_cast<sf::CircleShape*>(body->FindShapeCollider()->GetBaseShape());
		circle->setRadius(radius);
		constexpr float pointsCountConstant = 3.f;
		auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
		circle->setPointCount(pointsCount);

		sf::Color color(40, 170, 255, 200);
		auto outlineColor = color;
		outlineColor.a = 255;

		circle->setFillColor(color);
		circle->setOutlineColor(outlineColor);
		circle->setOutlineThickness(1);
		circle->setOrigin(utils::FindCenterOfMass(circle));
		auto minX = static_cast<int>(wallVisibleWidth + radius);
		auto maxX = static_cast<int>(screenSize.x - wallVisibleWidth - radius);
		auto minY = static_cast<int>(wallVisibleWidth + radius);
		auto maxY = static_cast<int>(screenSize.y - wallVisibleWidth - radius);
		auto x = static_cast<float>(minX + rand() % (maxX - minX));
		auto y = static_cast<float>(minY + rand() % (maxY - minY));
		circle->setPosition(sf::Vector2f{x, y});
		auto rb = body->RequireBehaviour<RigidBodyBehaviour>();
		rb->_mass = 3.14f * radius * radius;
		rb->_restitution = bodiesRestitution;
		body->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(0, true);
		scene->AddChild(std::move(body));
	}
	scene->AddChild(CreateFpsCounterNode());
	auto bodyPull = CreateBodyPullOverlay();
	scene->AddChild(std::move(bodyPull.node));
	EngineContext::Instance().SetBodyPullHandler(bodyPull.handler);
	return scene;
}

void TestEnvironment::ConfigureInput() {
	auto ei = &EngineContext::Instance();
	auto userInput = ei->GetUserInput();

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::R) {
				ei->SetScene(BuildScene());
			}
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
			sf::Vector2f pos(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y));
			if (auto scene = ei->GetScene()) {
				scene->DispatchTapAt(pos);
			}
			return;
		}
		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			sf::Vector2f mousePos(static_cast<float>(pressed->position.x), static_cast<float>(pressed->position.y));
			if (pressed->button == sf::Mouse::Button::Left) {
				bool tapConsumed = false;
				if (auto scene = ei->GetScene()) {
					tapConsumed = scene->DispatchTapAt(mousePos);
				}
				if (!tapConsumed) {
					ei->GetBodyPullHandler()->StartPull(mousePos, UserPullBehaviour::PullMode::FORCE);
				}
			}
			else if (pressed->button == sf::Mouse::Button::Right) {
				ei->GetBodyPullHandler()->StartPull(mousePos, UserPullBehaviour::PullMode::POSITION);
			}
			else if (pressed->button == sf::Mouse::Button::Middle) {
				ei->GetBodyPullHandler()->StartPull(mousePos, UserPullBehaviour::PullMode::VELOCITY);
			}
		}
		else if (event.is<sf::Event::MouseButtonReleased>()) {
			ei->GetBodyPullHandler()->StopPull();
		}
		else if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			ei->GetBodyPullHandler()->SetPullDestination(
			    sf::Vector2f(static_cast<float>(moved->position.x), static_cast<float>(moved->position.y)));
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			switch (key->code) {
			case sf::Keyboard::Key::Equal:
				ei->SetSimSpeedMultiplier(ei->GetSimSpeedMultiplier() * 2);
				break;
			case sf::Keyboard::Key::Hyphen:
				ei->SetSimSpeedMultiplier(ei->GetSimSpeedMultiplier() * 0.5f);
				break;
			case sf::Keyboard::Key::Num0:
				ei->SetSimPaused(!ei->IsSimPaused());
				break;
			default:
				break;
			}
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([](sf::Event event) {
		if (event.is<sf::Event::Closed>()) {
			std::exit(EXIT_SUCCESS);
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::G) {
				ei->GetPhysicsHandler()->SetGravityEnabled(!ei->GetPhysicsHandler()->IsGravityEnabled());
			}
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::D) {
				ei->SetDebugEnabled(!ei->IsDebugEnabled());
			}
		}
	}));
}
