#include "Env.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"

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

	std::shared_ptr<Scene> Env::BuildScene() {
		auto scene = std::make_shared<Scene>();
		return scene;
	}

} // namespace Demo1
