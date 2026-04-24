#include "Engine/App/EngineInterface.h"
#include "Environments/Pong/PongEnvironment.h"
#include "Environments/Test/TestEnvironment.h"
#include "Engine/App/UserInput.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>

#ifdef _CONSOLE
int main() {
#else
#define NOMINMAX
#include <windows.h>
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif
	_set_error_mode(_OUT_TO_MSGBOX);
	TestEnvironment env;
	// PongEnvironment env;
	env.Setup();

	EngineContext& engine = EngineContext::Instance();
	while (true) {
		auto window = engine.GetMainWindow();
		if (!window) {
			continue;
		}
		if (!window->isOpen()) {
			break;
		}
		auto scene = engine.GetScene();
		if (!scene) {
			continue;
		}

		engine.OnStartFrame();
		window->clear();

		while (auto ev = window->pollEvent()) {
			sf::Event event = *ev;
			if (event.is<sf::Event::Closed>()) {
				return EXIT_SUCCESS;
			}
			engine.GetUserInput()->HandleEvent(event);
		}

		auto dt = engine.GetSimDt();
		if (!engine.IsSimPaused()) {
			engine.GetPhysicsHandler()->Update(dt);
		}
		if (dt.asSeconds() > 0.1f) {
			dt = sf::seconds(0.1f);
		}
		scene->UpdateRec(dt);

		window->draw(*scene);
		window->display();
	}

	return EXIT_SUCCESS;
}
