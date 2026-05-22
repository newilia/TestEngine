#include "Demo1LaunchProfile.h"

#include "BallpitGame.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Core/TextureManager.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/SpriteVisual.h"
#include "PongGame.h"
#include "SolarSystemGame.h"
#include "TicTacToeGame.h"

#include <cstdlib>
#include <memory>

namespace Demo1 {
	void Demo1LaunchProfile::Setup() {
		auto& mainContext = Engine::MainContext::GetInstance();
		const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode::getFullscreenModes()[0], "Demo1");
		if (!mainWindow) {
			std::exit(EXIT_FAILURE);
		}
		mainContext.Init();
		Engine::Editor::GetInstance().Init();
		Utils::MaximizeWindow(*mainWindow);
		mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
		mainContext.GetPhysicsProcessor()->SetGravityEnabled(true);
		mainContext.SetScene(BuildScene());
		mainContext.SetVerticalSyncEnabled(false);
		EventHandlerBase::SubscribeForEvents();
	}

	void Demo1LaunchProfile::OnEvent(const sf::Event& event) {
		auto& mainContext = Engine::MainContext::GetInstance();

		if (auto e = event.getIf<sf::Event::KeyPressed>()) {
			if (e->code == sf::Keyboard::Key::R) {
				mainContext.SetScene(BuildScene());
			}
			else if (e->code == sf::Keyboard::Key::Escape) {
				std::exit(EXIT_SUCCESS);
			}
		}
	}

	shared_ptr<SceneNode> CreateBackgroundNode() {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto node = SceneNode::Create();
		auto sprite = make_shared<SpriteVisual>();
		auto texture = mainContext.GetTextureManager()->LoadTexture("resources/textures/fern_dark_green.jpg");
		if (texture) {
			sprite->SetTexture(*texture);
		}
		node->SetVisual(std::move(sprite));
		auto sorting = make_shared<RelativeSortingStrategy>();
		sorting->SetPriority(-10);
		node->SetSortingStrategy(std::move(sorting));
		node->SetLocalScale({2.f, 2.f});
		return node;
	}

	shared_ptr<Scene> Demo1LaunchProfile::BuildScene() {
		auto scene = make_shared<Scene>();
		{
			auto background = CreateBackgroundNode();
			background->SetName("Background");
			scene->GetRoot()->AddChild(background);
		}

		{
			const float fieldW = 500;
			const float fieldH = 900;
			const float platformWidth = 100;
			const float platformThickness = 20;
			const float wallsThickness = 50;
			const float ballRadius = 20;
			auto pongRoot =
			    CreatePongGameNode(fieldW, fieldH, platformWidth, platformThickness, wallsThickness, ballRadius);
			pongRoot->SetLocalPosition({800, 2000});
			scene->GetRoot()->AddChild(std::move(pongRoot));
		}

		{
			auto ticTacToe = CreateTicTacToeGameNode();
			ticTacToe->SetLocalPosition({500, 500});
			ticTacToe->SetLocalScale({1.5, 1.5});
			scene->GetRoot()->AddChild(std::move(ticTacToe));
		}

		{
			const float AquariumW = 800.f;
			const float AquariumH = 800.f;
			const float WallThickness = 60.f;
			const float BaseRadius = 15.f;
			const float RadiusVar = 0.f;
			const sf::Color BaseColor(255, 200, 80, 230);
			const float ColorVar = 0.f;
			const int BallCount = 250;
			const float BallRestitution = 0.95f;
			const float WallRestitution = 1.f;
			auto ballpit = CreateBallpitGameNode(AquariumW, AquariumH, WallThickness, BaseRadius, RadiusVar, BaseColor,
			    ColorVar, BallCount, BallRestitution, WallRestitution);
			ballpit->SetLocalPosition({2100, 700});
			scene->GetRoot()->AddChild(std::move(ballpit));
		}

		{
			auto solar = CreateSolarSystemGameNode();
			solar->SetLocalPosition({2700, 5000});
			scene->GetRoot()->AddChild(std::move(solar));
		}
		return scene;
	}

} // namespace Demo1
