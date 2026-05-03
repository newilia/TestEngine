#include "Env.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/TextureManager.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/SpriteVisual.h"
#include "PongGame.h"
#include "TicTacToeGame.h"

#include <cstdlib>
#include <memory>

namespace Demo1 {
	void Env::Setup() {
		auto& mainContext = Engine::MainContext::GetInstance();
		const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode::getFullscreenModes()[0], "Demo1");
		if (!mainWindow) {
			std::exit(EXIT_FAILURE);
		}
		Utils::MaximizeWindow(*mainWindow);
		mainContext.GetPhysicsProcessor()->SetGravity({0, 1000});
		mainContext.SetScene(BuildScene());
		mainContext.SetVerticalSyncEnabled(false);
		EventHandlerBase::SubscribeForEvents();
	}

	void Env::OnEvent(const sf::Event& event) {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto* window = mainContext.GetMainWindow();
		if (!window) {
			return;
		}

		if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
			const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, touch->position);
			if (auto scene = mainContext.GetScene()) {
				scene->DispatchTapAt(worldPos);
			}
			return;
		}

		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (pressed->button != sf::Mouse::Button::Left) {
				return;
			}
			const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, pressed->position);
			if (auto scene = mainContext.GetScene()) {
				scene->DispatchTapAt(worldPos);
			}
			return;
		}

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
		auto node = make_shared<SceneNode>();
		auto sprite = make_shared<SpriteVisual>();
		auto texture = mainContext.GetTextureManager()->LoadTexture("resources/textures/fern_dark_green.jpg");
		if (texture) {
			sprite->SetTexture(*texture);
		}
		node->SetVisual(std::move(sprite));
		auto sorting = make_shared<RelativeSortingStrategy>();
		sorting->SetPriority(-10);
		node->SetSortingStrategy(std::move(sorting));
		return node;
	}

	shared_ptr<Scene> Env::BuildScene() {
		auto scene = make_shared<Scene>();
		scene->AddChild(CreateBackgroundNode());

		const float fieldW = 500;
		const float fieldH = 900;
		const float platformWidth = 100;
		const float platformThickness = 50;
		const float wallsThickness = 50;
		auto pongRoot = CreatePongGameNode(fieldW, fieldH, platformWidth, platformThickness, wallsThickness);
		scene->AddChild(std::move(pongRoot));

		auto ticTacToe = CreateTicTacToeGameNode();
		ticTacToe->SetPosGlobal({80.f, 120.f});
		scene->AddChild(std::move(ticTacToe));
		return scene;
	}

} // namespace Demo1
