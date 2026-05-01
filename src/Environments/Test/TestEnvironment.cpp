#include "TestEnvironment.h"

#include "Engine/App/MainContext.h"
#include "Engine/App/UserInput.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/CollisionBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Tools/PullTool.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "fmt/format.h"

#include <memory>
#include <utility>

using std::make_shared;
using std::shared_ptr;

void TestEnvironment::Setup() {
	auto& engine = Engine::MainContext::GetInstance();
	const auto mainWindow = engine.CreateMainWindow(sf::VideoMode({1920, 1080}), "Test scene");
	if (!mainWindow) {
		std::exit(EXIT_FAILURE);
	}
	Utils::MaximizeWindow(*mainWindow);
	engine.GetPhysicsProcessor()->SetGravity({0, 1000});
	engine.SetScene(BuildScene());
	ConfigureInput();
}

std::shared_ptr<Scene> TestEnvironment::BuildScene() {
	auto scene = make_shared<Scene>();
	auto viewSize = Engine::MainContext::GetInstance().GetMainWindow()->getView().getSize();
	float commonRestitution = 0.99f;
	float commonFriction = 500.f;
	float commonAttraction = 100.f;
	bool isAttractive = true;

	/* walls */
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {{viewSize.x, wallActualWidth},
	                            {viewSize.x, wallActualWidth},
	                            {wallActualWidth, viewSize.y},
	                            {wallActualWidth, viewSize.y}};
	sf::Vector2f wallPositions[] = {{viewSize.x / 2, viewSize.y + wallOffset},
	                                {viewSize.x / 2, -wallOffset},
	                                {-wallOffset, viewSize.y / 2},
	                                {viewSize.x + wallOffset, viewSize.y / 2}};

	for (int i = 0; i < 4; ++i) {
		auto node = CreateShapeBodyNode<sf::RectangleShape>();
		node->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(0, true);
		node->SetName(wallNames[i]);

		auto* rect = dynamic_cast<sf::RectangleShape*>(node->FindShapeCollider()->GetBaseShape());
		rect->setSize(wallSizes[i]);
		rect->setOrigin(Utils::FindCenterOfMass(rect));
		rect->setPosition(wallPositions[i]);
		rect->setFillColor(sf::Color(30, 255, 30, 50));
		rect->setOutlineColor(sf::Color(30, 255, 30, 120));
		rect->setOutlineThickness(1.f);

		auto rb = node->RequireBehaviour<RigidBodyBehaviour>();
		rb->SetImmovable();
		rb->_restitution = commonRestitution;
		rb->_friction = commonFriction;

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->_attraction = 10000 * (isAttractive ? -1 : 1);
		node->AddBehaviour(std::move(fieldBeh));

		scene->AddChild(std::move(node));
	}

	/* circles */
	constexpr int circlesCount = 200;
	for (int i = 0; i < circlesCount; ++i) {
		auto node = CreateShapeBodyNode<sf::CircleShape>();
		node->SetName(fmt::format("circle_{}", i));
		node->RequireBehaviour<CollisionBehaviour>()->_collisionGroups.set(0, true);

		bool isAttractive = true;

		auto* circle = dynamic_cast<sf::CircleShape*>(node->FindShapeCollider()->GetBaseShape());
		float radius = 20.f;
		circle->setRadius(radius);
		// constexpr float pointsCountConstant = 3.f;
		// auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
		// circle->setPointCount(pointsCount);

		sf::Color color = isAttractive ? sf::Color(40, 170, 255, 200) : sf::Color(255, 100, 100, 200);
		auto outlineColor = color;
		outlineColor.a = 255;

		circle->setFillColor(color);
		circle->setOutlineColor(outlineColor);
		circle->setOutlineThickness(1);
		circle->setOrigin(Utils::FindCenterOfMass(circle));
		auto minX = static_cast<int>(wallVisibleWidth + radius);
		auto maxX = static_cast<int>(viewSize.x - wallVisibleWidth - radius);
		auto minY = static_cast<int>(wallVisibleWidth + radius);
		auto maxY = static_cast<int>(viewSize.y - wallVisibleWidth - radius);
		auto x = static_cast<float>(minX + rand() % (maxX - minX));
		auto y = static_cast<float>(minY + rand() % (maxY - minY));
		circle->setPosition(sf::Vector2f{x, y});

		auto rb = node->RequireBehaviour<RigidBodyBehaviour>();
		rb->_mass = 3.14f * radius * radius;
		rb->_restitution = commonRestitution;
		rb->_friction = commonFriction;

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->_attraction = commonAttraction * (isAttractive ? -1 : 1);
		node->AddBehaviour(std::move(fieldBeh));

		scene->AddChild(std::move(node));
	}
	auto pull = CreatePullVisualOverlay();
	scene->AddChild(std::move(pull.root));
	Engine::Editor::GetInstance().GetEditorToolManager().BindPullArrow(std::move(pull.arrowVisual));
	return scene;
}

void TestEnvironment::ConfigureInput() {
	auto ei = &Engine::MainContext::GetInstance();
	auto userInput = ei->GetUserInput();

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		auto* window = ei->GetMainWindow();
		if (!window) {
			return;
		}
		if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
			const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, touch->position);
			if (auto scene = ei->GetScene()) {
				scene->DispatchTapAt(worldPos);
			}
			return;
		}
		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (pressed->button != sf::Mouse::Button::Left) {
				return;
			}
			const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, pressed->position);
			if (auto scene = ei->GetScene()) {
				scene->DispatchTapAt(worldPos);
			}
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
				ei->GetPhysicsProcessor()->SetGravityEnabled(!ei->GetPhysicsProcessor()->IsGravityEnabled());
			}
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::D) {
				ei->SetDebugDrawEnabled(!ei->IsDebugDrawEnabled());
			}
		}
	}));

	userInput->AttachEventHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::R) {
				ei->SetScene(BuildScene());
			}
		}
	}));
}
