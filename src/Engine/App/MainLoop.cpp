#include "MainLoop.h"

#include "Engine/App/MainContext.h"
#include "Engine/Core/PeriodicTaskExecutor.h"
#include "Engine/Editor/Editor.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

// #include <stdlib.h>

namespace Engine {

	void MainLoop::Run() {
		PeriodicTaskExecutor tickExecutor(
		    []() {
			    return sf::seconds(1.f / MainContext::GetInstance().GetTargetTickRate());
		    },
		    [this](const sf::Time& dt) {
			    UpdateTick();
		    });

		PeriodicTaskExecutor presentExecutor(
		    []() {
			    MainContext& engine = Engine::MainContext::GetInstance();
			    if (!engine.IsFramerateLimitEnabled()) {
				    return sf::Time::Zero;
			    }
			    return sf::seconds(1.f / engine.GetFramerateLimit());
		    },
		    [this](const sf::Time& dt) {
			    PresentFrame();
		    });

		sf::Clock mainLoopClock;
		MainContext& engine = Engine::MainContext::GetInstance();
		{
			while (true) {
				const auto& window = engine.GetMainWindow();
				if (!window) {
					exit(EXIT_FAILURE);
				}
				if (!window->isOpen()) {
					break;
				}

				if (!PollAndDispatchEvents()) {
					break;
				}

				auto dt = mainLoopClock.restart();
				tickExecutor.Update(dt);
				presentExecutor.Update(dt);
			}
		}
	}

	bool MainLoop::PollAndDispatchEvents() {
		auto& engine = Engine::MainContext::GetInstance();
		const auto& window = engine.GetMainWindow();
		if (!window) {
			return false;
		}

		bool isImGuiInitialized = engine.IsImGuiInitialized();

		while (const auto& ev = window->pollEvent()) {
			if (!ev) {
				break;
			}
			if (!DispatchEvent(*ev)) {
				return false;
			}
		}
		return window->isOpen();
	}

	bool MainLoop::IsImGuiWantCaptureInput(const sf::Event& event) {
		if (!Engine::MainContext::GetInstance().IsImGuiInitialized()) {
			return false;
		}

		const ImGuiIO& io = ImGui::GetIO();
		if (event.is<sf::Event::MouseButtonPressed>() || event.is<sf::Event::MouseButtonReleased>() ||
		    event.is<sf::Event::MouseMoved>() || event.is<sf::Event::MouseWheelScrolled>() ||
		    event.is<sf::Event::MouseMovedRaw>() || event.is<sf::Event::TouchBegan>() ||
		    event.is<sf::Event::TouchMoved>() || event.is<sf::Event::TouchEnded>()) {
			return io.WantCaptureMouse;
		}
		if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::KeyReleased>()) {
			return io.WantCaptureKeyboard;
		}
		if (event.is<sf::Event::TextEntered>()) {
			return io.WantTextInput;
		}
		return true;
	}

	bool MainLoop::DispatchEvent(const sf::Event& event) {
		auto& engine = Engine::MainContext::GetInstance();
		bool isImGuiInitialized = engine.IsImGuiInitialized();
		const auto& window = engine.GetMainWindow();

		if (event.is<sf::Event::Closed>()) {
			window->close();
			return false;
		}

		if (isImGuiInitialized) {
			ImGui::SFML::ProcessEvent(*window, event);
		}

		if (const auto* resized = event.getIf<sf::Event::Resized>()) {
			engine.OnMainWindowResized(resized->size);
		}

		auto& editor = Editor::GetInstance();
		editor.OnEvent(event);

		if (IsImGuiWantCaptureInput(event)) {
			return true;
		}

		bool consumed = editor.GetEditorToolManager().ProcessEvent(event);
		if (!consumed) {
			engine.GetUserInput()->HandleEvent(event);
		}
		return true;
	}

	void MainLoop::UpdateTick() {
		auto& engine = Engine::MainContext::GetInstance();
		engine.OnStartUpdateTick();

		if (engine.IsSimPaused()) {
			return;
		}
		auto simulatedDt = engine.GetSimTickDt();
		if (auto scene = engine.GetScene()) {
			scene->UpdateRec(simulatedDt);
		}
		engine.GetPhysicsProcessor()->Update(simulatedDt);
	}

	bool MainLoop::PresentFrame() {
		auto& engine = Engine::MainContext::GetInstance();
		auto window = engine.GetMainWindow();

		if (!window) {
			return false;
		}
		auto scene = engine.GetScene();
		if (!scene) {
			return false;
		}

		engine.OnStartPresentFrame();
		auto dt = engine.GetFrameDt();

		window->clear();
		ImGui::SFML::Update(*window, dt);
		scene->NotifyPresentRec(dt);

		auto& editor = Engine::Editor::GetInstance();
		editor.GetEditorToolManager().OnPresent(dt);
		editor.Update(dt.asSeconds());
		editor.Draw();

		window->draw(*scene);
		ImGui::SFML::Render(*window);
		window->display();
		return true;
	}

} // namespace Engine
