#include "Env.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/TextureManager.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/SpriteVisual.h"

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
		EventHandlerBase::SubscribeForEvents();
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
		return scene;
	}

} // namespace Demo1
