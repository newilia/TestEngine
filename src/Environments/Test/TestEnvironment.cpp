#include "TestEnvironment.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
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

	mainContext.GetPhysicsProcessor()->SetGravityEnabled(true);
	mainContext.GetPhysicsProcessor()->SetSimulationSubsteps(2);
	mainContext.SetScene(BuildScene());

	EventHandlerBase::SubscribeForEvents();
}

void TestEnvironment::OnEvent(const sf::Event& event) {
	auto mainContext = &Engine::MainContext::GetInstance();

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
		else if (key->code == sf::Keyboard::Key::G) {
			mainContext->GetPhysicsProcessor()->SetGravityEnabled(
			    !mainContext->GetPhysicsProcessor()->IsGravityEnabled());
		}
		else if (key->code == sf::Keyboard::Key::R) {
			mainContext->SetScene(BuildScene());
		}
	}
}

std::shared_ptr<Scene> TestEnvironment::BuildScene() {
	auto scene = make_shared<Scene>();
	auto container = make_shared<SceneNode>();
	container->SetName("container");
	Engine::MainContext::GetInstance().FocusCameraOnNode(container);
	scene->GetRoot()->AddChild(container);

	constexpr sf::Vector2f boxSize{600, 600};
	constexpr float commonRestitution = 0.65f;
	constexpr float commonAttraction = 0.f;
	constexpr bool isAttractionPositive = false;

	/* walls */

	constexpr float wallsThickness = 200;
	constexpr sf::Color wallFillColor{30, 102, 30, 255};
	constexpr sf::Color wallOutlineColor{30, 150, 30, 255};

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	// Wall arrangement: bottom (y+), top (y-), left (x-), right (x+)
	sf::Vector2f wallSizes[] = {
	    {boxSize.x + 2 * wallsThickness, wallsThickness}, // bottom
	    {boxSize.x + 2 * wallsThickness, wallsThickness}, // top
	    {wallsThickness, boxSize.y},                      // left
	    {wallsThickness, boxSize.y}                       // right
	};
	// Positions are calculated so inner box is boxSize, walls wrap outside
	sf::Vector2f wallPositions[] = {
	    {0.0f, boxSize.y / 2.0f + wallsThickness / 2.0f},  // bottom
	    {0.0f, -boxSize.y / 2.0f - wallsThickness / 2.0f}, // top
	    {-boxSize.x / 2.0f - wallsThickness / 2.0f, 0.0f}, // left
	    {boxSize.x / 2.0f + wallsThickness / 2.0f, 0.0f}   // right
	};

	for (int i = 0; i < 4; ++i) {
		auto node = make_shared<SceneNode>();
		node->SetName(wallNames[i]);
		container->AddChild(std::move(node));

		auto rect = node->RequireVisual<RectangleShapeVisual>();
		rect->SetSize(wallSizes[i]);
		rect->SetOrigin(Utils::FindCenterOfMass(rect->GetShape()));
		Utils::SetLocalPosToWorld(node, wallPositions[i]);
		rect->SetFillColor(wallFillColor);
		rect->SetOutlineColor(wallOutlineColor);
		rect->SetOutlineThickness(0.0f);

		auto bodyBeh = node->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->GetInteractionGroups().set(0, true);
		bodyBeh->SetFixed(true);
		bodyBeh->SetRestitution(commonRestitution);

		if (commonAttraction != 0.f) {
			auto fieldBeh = std::make_shared<AttractiveBehaviour>();
			fieldBeh->SetAttraction(commonAttraction * (isAttractionPositive ? 1 : -1));
			node->AddBehaviour(std::move(fieldBeh));
		}

		Utils::AddLightReceiver(node.get(), 1.0f, false);
	}

	/* circles */

	constexpr int rowsCount = 15;
	constexpr int colsCount = 12;
	constexpr int circlesCount = rowsCount * colsCount;

	for (int i = 0; i < circlesCount; ++i) {
		const auto gridRow = i / colsCount;
		const auto gridCol = i % colsCount;
		int kind = 3.f * gridCol / colsCount;

		auto circleNode = make_shared<SceneNode>();
		circleNode->SetName(fmt::format("circle_{}", i));
		circleNode->RequireBehaviour<PhysicsBodyBehaviour>()->GetInteractionGroups().set(0, true);

		constexpr float radius = 15.f;
		constexpr auto kColor1 = sf::Color{200, 40, 40, 255};
		constexpr auto kColor2 = sf::Color{40, 200, 40, 255};
		constexpr auto kColor3 = sf::Color{40, 40, 200, 255};

		const auto color = (kind == 0) ? kColor1 : ((kind == 1) ? kColor2 : kColor3);
		const auto outlineColor = sf::Color(color.r / 2, color.g / 2, color.b / 2, 255);
		auto circle = circleNode->RequireVisual<CircleShapeVisual>();
		circle->SetRadius(radius);
		circle->SetFillColor(color);
		circle->SetOutlineColor(outlineColor);
		circle->SetOutlineThickness(0.0f);
		circle->SetOrigin(circle->GetLocalBounds().getCenter());

		// position in grid
		{
			const auto minX = -boxSize.x / 2;
			const auto maxX = boxSize.x / 2;
			const auto minY = -boxSize.y / 2;
			const auto maxY = boxSize.y / 2;
			const auto x = minX + (1 + gridCol) * (maxX - minX) / (colsCount + 1) + rand() % 2;
			const auto y = minY + (1 + gridRow) * (maxY - minY) / (rowsCount + 1);
			circleNode->GetLocalTransform()->SetPosition(sf::Vector2f{x, y});
		}

		auto bodyBeh = circleNode->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->SetMass(3.14f * radius * radius);
		bodyBeh->SetRestitution(commonRestitution);

		if (commonAttraction != 0.f) {
			auto fieldBeh = circleNode->RequireBehaviour<AttractiveBehaviour>();
			fieldBeh->SetAttraction(commonAttraction * (isAttractionPositive ? -1 : 1));
		}

		Utils::AddLightSource(circleNode.get(), 12, 5, color);
		Utils::AddLightReceiver(circleNode.get(), 1.f, true, radius);

		container->AddChild(std::move(circleNode));
	}

	return scene;
}
