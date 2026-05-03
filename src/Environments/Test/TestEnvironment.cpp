#include "TestEnvironment.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Tools/PullTool.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "fmt/format.h"

#include <memory>
#include <utility>

using std::make_shared;
using std::shared_ptr;

void TestEnvironment::Setup() {
	auto& mainContext = Engine::MainContext::GetInstance();
	const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode({1920, 1080}), "Test scene");
	if (!mainWindow) {
		std::exit(EXIT_FAILURE);
	}
	Utils::MaximizeWindow(*mainWindow);
	mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
	mainContext.SetScene(BuildScene());

	EventHandlerBase::SubscribeForEvents();
}

void TestEnvironment::OnEvent(const sf::Event& event) {
	auto mainContext = &Engine::MainContext::GetInstance();
	auto* window = mainContext->GetMainWindow();
	if (!window) {
		return;
	}

	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, touch->position);
		if (auto scene = mainContext->GetScene()) {
			scene->DispatchTapAt(worldPos);
		}
		return;
	}

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button != sf::Mouse::Button::Left) {
			return;
		}
		const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, pressed->position);
		if (auto scene = mainContext->GetScene()) {
			scene->DispatchTapAt(worldPos);
		}
	}

	if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
		if (key->code == sf::Keyboard::Key::Escape) {
			std::exit(EXIT_SUCCESS);
		}
		if (key->code == sf::Keyboard::Key::Equal) {
			mainContext->SetSimSpeedMultiplier(mainContext->GetSimSpeedMultiplier() * 2);
		}
		else if (key->code == sf::Keyboard::Key::Hyphen) {
			mainContext->SetSimSpeedMultiplier(mainContext->GetSimSpeedMultiplier() * 0.5f);
		}
		else if (key->code == sf::Keyboard::Key::Num0) {
			mainContext->SetSimPaused(!mainContext->IsSimPaused());
		}
		else if (key->code == sf::Keyboard::Key::G) {
			mainContext->GetPhysicsProcessor()->SetGravityEnabled(
			    !mainContext->GetPhysicsProcessor()->IsGravityEnabled());
		}
		else if (key->code == sf::Keyboard::Key::D) {
			mainContext->SetDebugDrawEnabled(!mainContext->IsDebugDrawEnabled());
		}
		else if (key->code == sf::Keyboard::Key::R) {
			mainContext->SetScene(BuildScene());
		}
	}
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
		auto node = make_shared<SceneNode>();
		node->SetVisual(std::make_shared<RectangleShapeVisual>());
		node->RequireBehaviour<PhysicsBodyBehaviour>()->GetCollisionGroups().set(0, true);

		node->SetName(wallNames[i]);

		auto* rect = dynamic_cast<sf::RectangleShape*>(node->FindPhysicsBody()->GetShape());
		rect->setSize(wallSizes[i]);
		rect->setOrigin(Utils::FindCenterOfMass(rect));
		rect->setPosition(wallPositions[i]);
		rect->setFillColor(sf::Color(30, 255, 30, 50));
		rect->setOutlineColor(sf::Color(30, 255, 30, 120));
		rect->setOutlineThickness(1.f);

		auto rb = node->RequireBehaviour<PhysicsBodyBehaviour>();
		rb->SetImmovable();
		rb->SetRestitution(commonRestitution);
		rb->SetFriction(commonFriction);

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->SetAttraction(10000 * (isAttractive ? -1 : 1));
		node->AddBehaviour(std::move(fieldBeh));

		scene->AddChild(std::move(node));
	}

	/* circles */
	constexpr int circlesCount = 200;
	for (int i = 0; i < circlesCount; ++i) {
		auto node = make_shared<SceneNode>();
		node->SetVisual(std::make_shared<CircleShapeVisual>());
		node->SetName(fmt::format("circle_{}", i));
		node->RequireBehaviour<PhysicsBodyBehaviour>()->GetCollisionGroups().set(0, true);

		bool isAttractive = true;

		auto* circle = dynamic_cast<sf::CircleShape*>(node->FindPhysicsBody()->GetShape());
		float radius = 20.f;
		circle->setRadius(radius);

		/* render optimization */
		constexpr float pointsCountConstant = 3.f;
		auto pointsCount = static_cast<size_t>(pointsCountConstant * (7 + radius / 8));
		circle->setPointCount(pointsCount);

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

		auto rb = node->RequireBehaviour<PhysicsBodyBehaviour>();
		rb->SetMass(3.14f * radius * radius);
		rb->SetRestitution(commonRestitution);
		rb->SetFriction(commonFriction);

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->SetAttraction(commonAttraction * (isAttractive ? -1 : 1));
		node->AddBehaviour(std::move(fieldBeh));

		scene->AddChild(std::move(node));
	}
	auto pull = CreatePullVisualOverlay();
	scene->AddChild(std::move(pull.root));
	Engine::Editor::GetInstance().GetEditorToolManager().BindPullArrow(std::move(pull.arrowVisual));
	return scene;
}
