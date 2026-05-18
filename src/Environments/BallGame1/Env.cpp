#include "Env.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/RadialUvWarpBehaviour.h"
#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Core/TextureManager.h"
#include "Engine/Core/Transform.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/VectorArrowVisual.h"
#include "GameControllerBehaviour.h"
#include "GunControllerBehaviour.h"

#include <cstdlib>
#include <memory>

namespace BallGame1 {
	void Env::Setup() {
		auto& mainContext = Engine::MainContext::GetInstance();
		const auto mainWindow =
		    mainContext.CreateMainWindow(sf::VideoMode::getFullscreenModes()[0], "Ball Game prototype");
		if (!mainWindow) {
			std::exit(EXIT_FAILURE);
		}
		EventHandlerBase::SubscribeForEvents();
		Utils::MaximizeWindow(*mainWindow);
		mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
		mainContext.GetPhysicsProcessor()->SetGravityEnabled(false);
		mainContext.SetVerticalSyncEnabled(true);
		mainContext.SetScene(BuildScene());

		auto backgroundCtx = mainContext.GetGameBackgroundContext();
		backgroundCtx->SetParallaxTextureBackground(
		    "resources/textures/backgrounds/plain_starfield_1.png", 1.f, 0.f, 0.15f, 3440);
	}

	void Env::OnEvent(const sf::Event& event) {
		if (auto e = event.getIf<sf::Event::KeyPressed>()) {
			auto& mainContext = Engine::MainContext::GetInstance();
			if (e->code == sf::Keyboard::Key::R) {
				mainContext.SetScene(BuildScene());
			}
			else if (e->code == sf::Keyboard::Key::Escape) {
				std::exit(EXIT_SUCCESS);
			}
		}
	}

	std::shared_ptr<Scene> Env::BuildScene() {
		auto scene = make_shared<Scene>();

		auto rootNode = SceneNode::Create();
		rootNode->SetName("Game");
		rootNode->SetLocalPosition({1700, 700});
		scene->GetRoot()->AddChild(rootNode);

		rootNode->AddChild(CreateFieldNode());

		auto gameController = rootNode->RequireBehaviour<GameControllerBehaviour>();
		gameController->SetShootVelocity(1500);
		gameController->SetCreateBallFunc([this] {
			return CreateBallNode();
		});

		auto gunNode = CreateGunNode();

		rootNode->AddChild(gunNode);
		auto scoreNode = CreateScoreNode();
		rootNode->AddChild(scoreNode);

		auto tmpNode = SceneNode::Create();
		rootNode->AddChild(tmpNode);

		gameController->SetFieldNode(tmpNode);
		gameController->SetGunNode(gunNode);
		gameController->SetScoreNode(scoreNode);
		gameController->StartNewGame();

		return scene;
	}

	/* stuff for game scene */

	std::shared_ptr<SceneNode> Env::CreateFieldNode() {
		const auto wallThickness = 100;
		const auto wallRestitution = 0.2f;

		const float hx = _fieldSize.x * 0.5f;
		const float hy = _fieldSize.y * 0.5f;

		const sf::Vector2f horizSize{_fieldSize.x + 2.f * wallThickness, wallThickness};
		const sf::Vector2f vertSize{wallThickness, _fieldSize.y + 2.f * wallThickness};

		const char* wallNames[] = {"Bottom", "Top", "Left", "Right"};
		const sf::Vector2f wallSizes[] = {horizSize, horizSize, vertSize, vertSize};
		const sf::Vector2f wallCentersLocal[] = {{0.f, hy + wallThickness * 0.5f}, {0.f, -hy - wallThickness * 0.5f},
		    {-hx - wallThickness * 0.5f, 0.f}, {hx + wallThickness * 0.5f, 0.f}};

		const sf::Color wallColor{120, 180, 220, 90};

		auto fieldNode = SceneNode::Create();
		fieldNode->SetName("Field");
		for (int i = 0; i < 4; ++i) {
			auto wallNode = SceneNode::Create();
			wallNode->SetName(wallNames[i]);
			auto rectVisual = wallNode->RequireVisual<RectangleShapeVisual>();
			auto* rectShape = rectVisual->GetShape();
			rectShape->setSize(wallSizes[i]);
			rectShape->setOrigin(rectShape->getGeometricCenter());
			rectShape->setFillColor(wallColor);

			fieldNode->AddChild(wallNode);
			wallNode->SetLocalPosition(wallCentersLocal[i]);

			auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
			bodyBeh->SetFixed(true);
			bodyBeh->SetRestitution(wallRestitution);
		}
		return fieldNode;
	}

	std::shared_ptr<SceneNode> Env::CreateGunNode() {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto node = SceneNode::Create();
		node->SetLocalPosition({0, _fieldSize.y * 0.5f});
		node->SetName("Gun");

		auto rectangle = node->RequireVisual<RectangleShapeVisual>();
		rectangle->SetSize({40, 200});
		rectangle->SetOrigin({20, 200});
		rectangle->SetFillColor(sf::Color::White);

		auto arrowNode = SceneNode::Create();
		arrowNode->SetName("Arrow");
		auto arrowVisual = arrowNode->RequireVisual<VectorArrowVisual>();
		arrowVisual->SetStartPos({0, 0});
		arrowVisual->SetEndPos({0, -100});
		arrowVisual->SetFillColor(sf::Color::White);
		node->AddChild(arrowNode);

		auto gunController = node->RequireBehaviour<GunControllerBehaviour>();
		gunController->SetRotationSpeed(2.5f);
		constexpr auto halfAngle = sf::degrees(65);
		gunController->SetRotationLimits(-halfAngle, halfAngle);

		return node;
	}

	std::shared_ptr<SceneNode> Env::CreateScoreNode() {
		auto node = SceneNode::Create();
		node->SetName("Score");
		return node;
	}

	std::shared_ptr<SceneNode> Env::CreateBallNode() {
		constexpr float mass = 500.0f;
		constexpr float restitution = 0.7f;
		constexpr float radius = 20.0f;
		constexpr auto color = sf::Color(128, 128, 128, 255);
		constexpr float attraction = 50000;
		constexpr float warpRadius = 500;
		constexpr float warpIntensity = 0.05;

		auto ballNode = SceneNode::Create();
		ballNode->SetName("Ball");

		auto visual = ballNode->RequireVisual<CircleShapeVisual>();
		visual->SetRadius(radius);
		visual->SetFillColor(color);

		auto body = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
		body->SetMass(mass);
		body->SetRestitution(restitution);

		if (attraction != 0.f) {
			auto attractive = ballNode->RequireBehaviour<AttractiveBehaviour>();
			attractive->SetAttraction(attraction);
		}

		/*auto warp = ballNode->RequireBehaviour<RadialUvWarpBehaviour>();
		warp->SetInfluenceRadius(warpRadius);
		warp->SetIntensity(warpIntensity);*/

		return ballNode;
	}
} // namespace BallGame1
