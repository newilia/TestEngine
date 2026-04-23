#include "EngineInterface.h"
#include "Pong/PongEnvironment.h"
#include "UserInput.h"

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
	// TestEnvironment env;
	PongEnvironment env;
	env.setup();

	EngineContext& engine = EngineContext::instance();
	while (true) {
		auto window = engine.getMainWindow();
		if (!window) {
			continue;
		}
		if (!window->isOpen()) {
			break;
		}
		auto scene = engine.getScene();
		if (!scene) {
			continue;
		}

		engine.onStartFrame();
		window->clear();

		while (auto ev = window->pollEvent()) {
			sf::Event event = *ev;
			if (event.is<sf::Event::Closed>()) {
				return EXIT_SUCCESS;
			}
			engine.getUserInput()->handleEvent(event);
		}

		auto dt = engine.getSimDt();
		if (!engine.isSimPaused()) {
			engine.getPhysicsHandler()->update(dt);
		}
		if (dt.asSeconds() > 0.1f) {
			dt = sf::seconds(0.1f);
		}
		scene->updateRec(dt);

		window->draw(*scene);
		window->draw(*engine.getBodyPullHandler());
		window->display();
	}

	return EXIT_SUCCESS;
}
