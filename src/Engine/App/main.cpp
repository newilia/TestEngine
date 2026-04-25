#include "Engine/App/EngineInterface.h"
#include "Engine/App/UserInput.h"
#include "Engine/Editor/Editor.h"
#include "Environments/Pong/PongEnvironment.h"
#include "Environments/Test/TestEnvironment.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

namespace {

	// Uses ImGui IO flags from the previous frame (after the last ImGui::SFML::Update), which is
	// the usual pattern for deciding whether application code should see mouse/keyboard events.
	[[nodiscard]] bool ShouldForwardEventToGame(const sf::Event& event) {
		const ImGuiIO& io = ImGui::GetIO();
		if (event.is<sf::Event::MouseButtonPressed>() || event.is<sf::Event::MouseButtonReleased>() ||
		    event.is<sf::Event::MouseMoved>() || event.is<sf::Event::MouseMovedRaw>() ||
		    event.is<sf::Event::MouseWheelScrolled>() || event.is<sf::Event::TouchBegan>() ||
		    event.is<sf::Event::TouchMoved>() || event.is<sf::Event::TouchEnded>()) {
			return !io.WantCaptureMouse;
		}
		if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::KeyReleased>()) {
			return !io.WantCaptureKeyboard;
		}
		if (event.is<sf::Event::TextEntered>()) {
			return !io.WantTextInput;
		}
		return true;
	}

} // namespace

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
	bool imguiInitialized = false;
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

		if (!imguiInitialized) {
			if (!ImGui::SFML::Init(*window)) {
				return EXIT_FAILURE;
			}
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			imguiInitialized = true;
		}

		engine.OnStartFrame();
		window->clear();

		while (auto ev = window->pollEvent()) {
			ImGui::SFML::ProcessEvent(*window, *ev);
			sf::Event event = *ev;
			if (event.is<sf::Event::Closed>()) {
				window->close();
				break;
			}
			Engine::Editor::GetInstance()->OnEvent(event);
			if (ShouldForwardEventToGame(event)) {
				engine.GetUserInput()->HandleEvent(event);
			}
		}

		if (!window->isOpen()) {
			break;
		}

		ImGui::SFML::Update(*window, engine.GetFrameDt());

		auto dt = engine.GetSimDt();
		if (!engine.IsSimPaused()) {
			engine.GetPhysicsHandler()->Update(dt);
		}
		if (dt.asSeconds() > 0.1f) {
			dt = sf::seconds(0.1f);
		}
		scene->UpdateRec(dt);

		Engine::Editor::GetInstance()->Update(engine.GetFrameDt().asSeconds());
		Engine::Editor::GetInstance()->Draw();

		window->draw(*scene);
		ImGui::SFML::Render(*window);
		window->display();
	}

	if (imguiInitialized) {
		ImGui::SFML::Shutdown();
	}

	return EXIT_SUCCESS;
}
