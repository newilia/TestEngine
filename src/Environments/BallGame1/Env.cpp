#include "Env.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/TextureManager.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/VectorArrowVisual.h"
#include "fmt/format.h"

#include <cstdlib>
#include <memory>

namespace BallGame1 {
	void Env::Setup() {
		auto& mainContext = Engine::MainContext::GetInstance();
		const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode::getFullscreenModes()[0], "BallGame1");
		if (!mainWindow) {
			std::exit(EXIT_FAILURE);
		}
		Utils::MaximizeWindow(*mainWindow);
		mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
		mainContext.GetPhysicsProcessor()->SetGravityEnabled(true);
		mainContext.SetScene(BuildScene());
		mainContext.SetVerticalSyncEnabled(false);
		EventHandlerBase::SubscribeForEvents();
	}

	void Env::OnEvent(const sf::Event& event) {
		if (auto e = event.getIf<sf::Event::KeyPressed>()) {
			auto& mainContext = Engine::MainContext::GetInstance();
			if (e->code == sf::Keyboard::Key::R) {
				mainContext.SetScene(BuildScene());
			}
		}
	}

	std::shared_ptr<Scene> Env::BuildScene() {
		auto scene = make_shared<Scene>();
		{
			auto background = CreateBackgroundNode();
			scene->AddChild(background);
		}
		{
			auto fieldNode = CreateFieldNode();
			scene->AddChild(fieldNode);
		}
		{
			auto ballNode = CreateBallNode();
			scene->AddChild(ballNode);
		}
		{
			auto platformNode = CreateStartNode();
			scene->AddChild(platformNode);
		}
		{
			auto scoreNode = CreateScoreNode();
			scene->AddChild(scoreNode);
		}
		return scene;
	}

	std::shared_ptr<SceneNode> Env::CreateBackgroundNode() {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto node = make_shared<SceneNode>();
		node->SetName("Background");
		auto sprite = make_shared<SpriteVisual>();
		auto texture = mainContext.GetTextureManager()->LoadTexture("resources/textures/ballgame1_background.jpg");
		if (texture) {
			sprite->SetTexture(*texture);
		}
		node->SetVisual(std::move(sprite));
		auto sorting = make_shared<RelativeSortingStrategy>();
		sorting->SetPriority(-10);
		return node;
	}

	std::shared_ptr<SceneNode> Env::CreateFieldNode() {
		const auto wallThickness = 100;
		const auto wallRestitution = 0.9;

		const float hx = _fieldSize.x * 0.5f;
		const float hy = _fieldSize.y * 0.5f;

		const sf::Vector2f horizSize{_fieldSize.x + 2.f * wallThickness, wallThickness};
		const sf::Vector2f vertSize{wallThickness, _fieldSize.y + 2.f * wallThickness};

		const char* wallNames[] = {"Bottom", "Top", "Left", "Right"};
		const sf::Vector2f wallSizes[] = {horizSize, horizSize, vertSize, vertSize};
		const sf::Vector2f wallCentersLocal[] = {{0.f, hy + wallThickness * 0.5f},
		                                         {0.f, -hy - wallThickness * 0.5f},
		                                         {-hx - wallThickness * 0.5f, 0.f},
		                                         {hx + wallThickness * 0.5f, 0.f}};

		const sf::Color wallColor{120, 180, 220, 90};

		auto fieldNode = make_shared<SceneNode>();
		fieldNode->SetName("Field");
		for (int i = 0; i < 4; ++i) {
			auto wallNode = make_shared<SceneNode>();
			wallNode->SetName(wallNames[i]);
			auto rectVisual = make_shared<RectangleShapeVisual>();
			wallNode->SetVisual(rectVisual);
			auto* rectShape = rectVisual->GetShape();
			rectShape->setSize(wallSizes[i]);
			rectShape->setOrigin(rectShape->getGeometricCenter());
			rectShape->setFillColor(wallColor);

			fieldNode->AddChild(wallNode);
			wallNode->GetLocalTransform()->SetPosition(wallCentersLocal[i]);

			auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
			bodyBeh->SetImmovable(true);
			bodyBeh->SetRestitution(wallRestitution);
		}
		return fieldNode;
	}

	std::shared_ptr<SceneNode> Env::CreateBallNode() {
		auto node = make_shared<SceneNode>();
		node->SetName("Ball");
		return node;
	}

	std::shared_ptr<SceneNode> Env::CreateStartNode() {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto node = make_shared<SceneNode>();
		node->SetName("Start");

		auto arrowNode = make_shared<SceneNode>();
		arrowNode->SetName("Arrow");
		arrowNode->GetLocalTransform()->SetPosition({0, _fieldSize.y * 0.5f});
		auto arrowVisual = make_shared<VectorArrowVisual>();
		arrowVisual->SetStartPos({0, 0});
		arrowVisual->SetEndPos({0, -100});
		arrowVisual->SetColor(sf::Color::White);
		arrowNode->SetVisual(std::move(arrowVisual));
		node->AddChild(arrowNode);
		return node;
	}

	std::shared_ptr<SceneNode> Env::CreateScoreNode() {
		auto node = make_shared<SceneNode>();
		node->SetName("Score");
		return node;
	}
} // namespace BallGame1
