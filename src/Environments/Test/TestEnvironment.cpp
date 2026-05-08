#include "TestEnvironment.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/PointLightBehaviour.h"
#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
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

	mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
	mainContext.GetPhysicsProcessor()->SetGravityEnabled(false);
	mainContext.GetPhysicsProcessor()->SetMotionSubsteps(2);
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
	auto boxSize = sf::Vector2f(600, 600);
	constexpr float commonRestitution = 0.65f;
	constexpr float commonAttraction = 100.f;
	constexpr bool isAttractionPositive = false;

	/* light stuff */

	const auto AddLightSource = [](SceneNode* node, float intensity, float radius, sf::Color color) {
		auto pl = node->RequireBehaviour<PointLightBehaviour>();
		pl->SetLightColor(color);
		pl->SetIntensity(intensity);
		pl->SetRadius(radius);
	};

	const auto AddLightReceiver = [](SceneNode* node, float diffusion, bool bevelEmboss, float bevelWidth = 0.f) {
		auto recv = node->RequireBehaviour<ShapeLightReceiverBehaviour>();
		recv->SetBevelEmbossMode(bevelEmboss);
		recv->SetBevelWidth(bevelWidth);
		recv->SetDiffusion(diffusion);
		recv->SetEaseOutCirc(false);
	};

	/* walls */

	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	constexpr sf::Color wallFillColor{30, 102, 30, 255};
	constexpr sf::Color wallOutlineColor{30, 150, 30, 255};

	std::string wallNames[] = {"bottom", "top", "left", "right"};
	sf::Vector2f wallSizes[] = {{boxSize.x, wallActualWidth}, {boxSize.x, wallActualWidth},
	    {wallActualWidth, boxSize.y}, {wallActualWidth, boxSize.y}};
	sf::Vector2f wallPositions[] = {{boxSize.x / 2, boxSize.y + wallOffset}, {boxSize.x / 2, -wallOffset},
	    {-wallOffset, boxSize.y / 2}, {boxSize.x + wallOffset, boxSize.y / 2}};

	for (int i = 0; i < 4; ++i) {
		auto node = make_shared<SceneNode>();
		node->SetName(wallNames[i]);
		scene->GetRoot()->AddChild(std::move(node));

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

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->SetAttraction(commonAttraction * (isAttractionPositive ? -1 : 1));
		node->AddBehaviour(std::move(fieldBeh));

		AddLightReceiver(node.get(), 0.0f, false);
	}

	/* circles */

	constexpr int rowsCount = 15;
	constexpr int colsCount = 10;
	constexpr int circlesCount = rowsCount * colsCount;

	for (int i = 0; i < circlesCount; ++i) {
		const auto gridRow = i / colsCount;
		const auto gridCol = i % colsCount;
		int kind = std::round(gridCol * 3.f / colsCount);

		auto node = make_shared<SceneNode>();
		auto circle = node->RequireVisual<CircleShapeVisual>();
		node->SetName(fmt::format("circle_{}", i));
		node->RequireBehaviour<PhysicsBodyBehaviour>()->GetInteractionGroups().set(0, true);

		constexpr float radius = 20.f;
		constexpr auto kColor1 = sf::Color{200, 40, 40, 255};
		constexpr auto kColor2 = sf::Color{40, 200, 40, 255};
		constexpr auto kColor3 = sf::Color{40, 40, 200, 255};

		const auto color = (kind == 0) ? kColor1 : ((kind == 1) ? kColor2 : kColor3);
		const auto outlineColor = sf::Color(color.r / 2, color.g / 2, color.b / 2, 255);
		circle->SetRadius(radius);
		circle->SetFillColor(color);
		circle->SetOutlineColor(outlineColor);
		circle->SetOutlineThickness(0.0f);
		circle->SetOrigin(circle->GetLocalBounds().getCenter());

		// position in grid
		{
			const auto minX = wallVisibleWidth + radius;
			const auto maxX = boxSize.x - wallVisibleWidth - radius;
			const auto minY = wallVisibleWidth + radius;
			const auto maxY = boxSize.y - wallVisibleWidth - radius;
			const auto x = minX + gridCol * (maxX - minX) / colsCount;
			const auto y = minY + gridRow * (maxY - minY) / rowsCount;
			Utils::SetLocalPosToWorld(node, sf::Vector2f{x, y});
		}

		auto bodyBeh = node->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->SetMass(3.14f * radius * radius);
		bodyBeh->SetRestitution(commonRestitution);

		auto fieldBeh = node->RequireBehaviour<AttractiveBehaviour>();
		fieldBeh->SetAttraction(commonAttraction * (isAttractionPositive ? -1 : 1));

		AddLightSource(node.get(), 20, 3, color);
		AddLightReceiver(node.get(), 0.7f, true, 25);

		scene->GetRoot()->AddChild(std::move(node));
	}

	return scene;
}
