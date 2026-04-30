#include "Engine/App/EngineContext.h"
#include "Engine/App/UserInput.h"
#include "Engine/Core/PeriodicTaskExecutor.h"
#include "Engine/Editor/Editor.h"
#include "Environments/Pong/PongEnvironment.h"
#include "Environments/Test/TestEnvironment.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

#include <cctype>
#include <string_view>

namespace {

	enum class AppEnvironmentKind
	{
		Test,
		Pong
	};

	[[nodiscard]] bool EqualsIgnoreCaseAscii(std::string_view a, std::string_view b) {
		if (a.size() != b.size()) {
			return false;
		}
		for (size_t i = 0; i < a.size(); ++i) {
			if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] AppEnvironmentKind ParseEnvFromArgv(int argc, char** argv) {
		constexpr std::string_view prefix = "--env=";
		for (int i = 1; i < argc; ++i) {
			std::string_view arg(argv[i]);
			if (arg.size() >= prefix.size() && arg.substr(0, prefix.size()) == prefix) {
				const std::string_view value = arg.substr(prefix.size());
				if (EqualsIgnoreCaseAscii(value, "pong")) {
					return AppEnvironmentKind::Pong;
				}
				if (EqualsIgnoreCaseAscii(value, "test")) {
					return AppEnvironmentKind::Test;
				}
			}
		}
		return AppEnvironmentKind::Test;
	}

	[[nodiscard]] AppEnvironmentKind ParseEnvFromCmdLine(std::string_view fullLine) {
		constexpr std::string_view key = "--env=";
		size_t pos = 0;
		while (pos < fullLine.size()) {
			const size_t found = fullLine.find(key, pos);
			if (found == std::string_view::npos) {
				break;
			}
			size_t valStart = found + key.size();
			size_t valEnd = valStart;
			while (valEnd < fullLine.size() && !std::isspace(static_cast<unsigned char>(fullLine[valEnd]))) {
				++valEnd;
			}
			const std::string_view value = fullLine.substr(valStart, valEnd - valStart);
			if (EqualsIgnoreCaseAscii(value, "pong")) {
				return AppEnvironmentKind::Pong;
			}
			if (EqualsIgnoreCaseAscii(value, "test")) {
				return AppEnvironmentKind::Test;
			}
			pos = valEnd;
		}
		return AppEnvironmentKind::Test;
	}

	void RunAppEnvironment(AppEnvironmentKind kind) {
		switch (kind) {
		case AppEnvironmentKind::Test:
			TestEnvironment::Setup();
			return;
		case AppEnvironmentKind::Pong: {
			PongEnvironment pong;
			pong.Setup();
			return;
		}
		}
	}

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

	[[nodiscard]] float GetPresentationPeriodSec(const EngineContext& engine) {
		const std::uint32_t limit = engine.GetFramerateLimit();
		if (limit > 0u) {
			return 1.f / static_cast<float>(limit);
		}
		return 1.f / 60.f;
	}

	/// Returns false if the window was closed. ImGui-SFML APIs require Init first (`imguiSfmlReady`).
	[[nodiscard]] bool PollAndDispatchEvents(EngineContext& engine, sf::RenderWindow& window, bool imguiSfmlReady) {
		while (auto ev = window.pollEvent()) {
			if (imguiSfmlReady) {
				ImGui::SFML::ProcessEvent(window, *ev);
			}
			sf::Event event = *ev;
			if (event.is<sf::Event::Closed>()) {
				window.close();
				return false;
			}
			if (const auto* resized = event.getIf<sf::Event::Resized>()) {
				const auto sz = resized->size;
				window.setView(
				    sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(sz.x), static_cast<float>(sz.y)})));
			}
			Engine::Editor::GetInstance().OnEvent(event);
			const bool forwardToGame = !imguiSfmlReady || ShouldForwardEventToGame(event);
			bool consumed = false;
			if (forwardToGame) {
				consumed = Engine::Editor::GetInstance().GetEditorToolManager().ProcessEvent(event);
			}
			if (forwardToGame && !consumed) {
				engine.GetUserInput()->HandleEvent(event);
			}
		}
		return window.isOpen();
	}

	[[nodiscard]] bool UpdateTick(EngineContext& engine, const std::shared_ptr<Scene>& scene, const sf::Time& dt) {
		if (engine.IsSimPaused()) {
			return false;
		}
		engine.OnStartUpdateTick();
		auto simulatedDt = engine.GetSimTickDt();

		auto* ph = engine.GetPhysicsProcessor().get();
		scene->UpdateRec(simulatedDt);
		ph->Update(simulatedDt);
		return true;
	}

	[[nodiscard]] bool PresentFrame(const sf::Time& dt, EngineContext& engine, sf::RenderWindow& window,
	                                const std::shared_ptr<Scene>& scene, bool& imguiInitialized) {
		/* TODO move out of here */
		{
			if (!imguiInitialized) {
				if (!ImGui::SFML::Init(window)) {
					return false;
				}
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
				imguiInitialized = true;
			}
		}

		engine.OnStartPresentFrame();
		window.clear();
		ImGui::SFML::Update(window, engine.GetFrameDt());

		scene->NotifyPresentRec(engine.GetFrameDt());

		Engine::Editor::GetInstance().GetEditorToolManager().OnPresent(engine.GetFrameDt());

		Engine::Editor::GetInstance().Update(engine.GetFrameDt().asSeconds());
		Engine::Editor::GetInstance().Draw();

		window.draw(*scene);
		ImGui::SFML::Render(window);
		window.display();
		return true;
	}

} // namespace

#ifdef _CONSOLE
int main(int argc, char** argv) {
#else
#define NOMINMAX
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                   _In_ int nShowCmd) {
#endif
	_set_error_mode(_OUT_TO_MSGBOX);

#ifdef _CONSOLE
	RunAppEnvironment(ParseEnvFromArgv(argc, argv));
#else
	RunAppEnvironment(ParseEnvFromCmdLine(lpCmdLine ? std::string_view(lpCmdLine) : std::string_view{}));
#endif

	EngineContext& engine = EngineContext::GetInstance();
	bool isImGuiInitialized = false;

	const auto targetTickPeriod = [&engine]() {
		auto period = sf::seconds(1.f / engine.GetTargetTickRateHz());
		return period;
	};
	PeriodicTaskExecutor tickExecutor(targetTickPeriod, [&engine, &isImGuiInitialized](const sf::Time& dt) {
		auto scene = engine.GetScene();
		if (scene) {
			(void)UpdateTick(engine, scene, dt);
		}
	});

	const auto targetFramePeriod = [&engine]() {
		auto period = sf::seconds(1.f / engine.GetFramerateLimit());
		return period;
	};
	PeriodicTaskExecutor presentExecutor(targetFramePeriod, [&engine, &isImGuiInitialized](const sf::Time& dt) {
		auto window = engine.GetMainWindow();
		auto scene = engine.GetScene();
		if (window && scene) {
			(void)PresentFrame(dt, engine, *window, scene, isImGuiInitialized);
		}
	});

	sf::Clock mainLoopClock;

	while (true) {
		auto window = engine.GetMainWindow();
		if (!window) {
			return EXIT_FAILURE;
		}
		if (!window->isOpen()) {
			break;
		}

		if (!PollAndDispatchEvents(engine, *window, isImGuiInitialized)) {
			break;
		}

		auto dt = mainLoopClock.restart();
		tickExecutor.Update(dt);
		presentExecutor.Update(dt);
	}

	if (isImGuiInitialized) {
		ImGui::SFML::Shutdown();
	}

	return EXIT_SUCCESS;
}
