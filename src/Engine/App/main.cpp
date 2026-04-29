#include "Engine/App/EngineContext.h"
#include "Engine/App/UserInput.h"
#include "Engine/Editor/Editor.h"
#include "Environments/Pong/PongEnvironment.h"
#include "Environments/Test/TestEnvironment.h"

#include <SFML/Graphics.hpp>

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>

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
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                   _In_ int nShowCmd) {
#endif
	_set_error_mode(_OUT_TO_MSGBOX);
	TestEnvironment env;
	// PongEnvironment env;
	env.Setup();

	EngineContext& engine = EngineContext::GetInstance();
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
			if (const auto* resized = event.getIf<sf::Event::Resized>()) {
				const auto sz = resized->size;
				window->setView(
				    sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(sz.x), static_cast<float>(sz.y)})));
			}
			Engine::Editor::GetInstance().OnEvent(event);
			if (ShouldForwardEventToGame(event)) {
				engine.GetUserInput()->HandleEvent(event);
			}
		}

		if (!window->isOpen()) {
			break;
		}

		ImGui::SFML::Update(*window, engine.GetFrameDt());

		engine.BeginLogicFrame();

		const std::uint32_t tickHz = engine.GetTargetTickRateHz();
		unsigned logicTicks = 0;

		if (!engine.IsSimPaused()) {
			auto* ph = engine.GetPhysicsHandler().get();
			if (tickHz == 0) {
				const float rawSec = engine.GetRawFrameDt().asSeconds();
				const float dtSec = std::min(0.1f, rawSec * engine.GetSimSpeedMultiplier());
				const sf::Time dt = sf::seconds(dtSec);
				engine.SetLastLogicStepDt(dt);
				scene->UpdateRec(dt);
				ph->Update(dt);
				logicTicks = 1;
			}
			else {
				const double stepSec = 1.0 / static_cast<double>(tickHz);
				constexpr unsigned kMaxSimTicksPerFrame = 16u;
				while (logicTicks < kMaxSimTicksPerFrame && engine.TryConsumeLogicAccumulator(stepSec)) {
					const sf::Time dt = sf::seconds(static_cast<float>(stepSec));
					engine.SetLastLogicStepDt(dt);
					scene->UpdateRec(dt);
					ph->Update(dt);
					++logicTicks;
				}
			}
		}
		engine.SetLogicTicksLastFrame(logicTicks);

		const sf::Time realFrameDt = engine.GetRawFrameDt();
		scene->NotifyPresentRec(realFrameDt);

		Engine::Editor::GetInstance().Update(engine.GetFrameDt().asSeconds());
		Engine::Editor::GetInstance().Draw();

		window->draw(*scene);
		ImGui::SFML::Render(*window);
		window->display();
	}

	if (imguiInitialized) {
		ImGui::SFML::Shutdown();
	}

	return EXIT_SUCCESS;
}
