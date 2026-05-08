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

#include <limits>
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
	mainContext.GetPhysicsProcessor()->SetGravityEnabled(true);
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
	auto boxSize = sf::Vector2f(1000, 800);
	constexpr float commonRestitution = 0.65f;
	constexpr float commonAttraction = 100.f;
	constexpr bool isAttractionPositive = false;

	/* walls */

	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;

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
		rect->SetFillColor(sf::Color(30, 255, 30, 50));
		rect->SetOutlineColor(sf::Color(30, 255, 30, 120));
		rect->SetOutlineThickness(1.f);

		auto bodyBeh = node->RequireBehaviour<PhysicsBodyBehaviour>();
		bodyBeh->GetInteractionGroups().set(0, true);
		bodyBeh->SetFixed(true);
		bodyBeh->SetRestitution(commonRestitution);

		auto fieldBeh = std::make_shared<AttractiveBehaviour>();
		fieldBeh->SetAttraction(commonAttraction * (isAttractionPositive ? -1 : 1));
		node->AddBehaviour(std::move(fieldBeh));
	}

	/* light stuff */

	const auto AddLightSource = [](SceneNode* node) {
		auto pl = node->RequireBehaviour<PointLightBehaviour>();
		pl->SetLightColor(sf::Color(160, 210, 255, 255));
		pl->SetIntensity(0.1);
		pl->SetRadius(std::numeric_limits<float>::max());
	};

	const auto AddLightReceiver = [](SceneNode* node, int type) {
		auto recv = node->RequireBehaviour<ShapeLightReceiverBehaviour>();
		if (type % 3 == 0) {
			recv->SetBevelEmbossMode(false);
			recv->SetDiffusion(0.35f);
		}
		else if (type % 3 == 1) {
			recv->SetBevelEmbossMode(true);
			recv->SetBevelWidth(18.f);
			recv->SetDiffusion(0.5f);
		}
		else if (type % 3 == 2) {
			recv->SetBevelEmbossMode(true);
			recv->SetEaseOutCirc(false);
			recv->SetBevelWidth(10.f);
			recv->SetDiffusion(0.75f);
		}
	};

	/* circles */

	constexpr int rowsCount = 15;
	constexpr int colsCount = 10;
	constexpr int circlesCount = rowsCount * colsCount;

	for (int i = 0; i < circlesCount; ++i) {
		auto node = make_shared<SceneNode>();
		auto circle = node->RequireVisual<CircleShapeVisual>();
		node->SetName(fmt::format("circle_{}", i));
		node->RequireBehaviour<PhysicsBodyBehaviour>()->GetInteractionGroups().set(0, true);

		constexpr float radius = 20.f;
		static const auto kAttractiveCircleColor = sf::Color{40, 170, 255, 200};
		static const auto kRepulsiveCircleColor = sf::Color{255, 100, 100, 200};
		const auto color = isAttractionPositive ? kAttractiveCircleColor : kRepulsiveCircleColor;
		const auto outlineColor = sf::Color(color.r, color.g, color.b, 255);
		circle->SetRadius(radius);
		circle->SetFillColor(color);
		circle->SetOutlineColor(outlineColor);
		circle->SetOutlineThickness(1);
		circle->SetOrigin(circle->GetLocalBounds().getCenter());

		const auto gridRow = i / colsCount;
		const auto gridCol = i % colsCount;

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

		AddLightSource(node.get());

		AddLightReceiver(node.get(), gridCol / colsCount);

		scene->GetRoot()->AddChild(std::move(node));
	}

	return scene;
}
