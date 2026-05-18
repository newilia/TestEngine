#include "MainLoop.h"

#include "Engine/Core/EventsDispatcher.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/PeriodicTaskExecutor.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Render/ViewportFullscreenEffect.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

// #include <stdlib.h>

namespace Engine {

	void MainLoop::SyncImGuiSfmlWindowFocus(sf::Window& window) {
		if (!MainContext::GetInstance().IsImGuiInitialized()) {
			return;
		}

		const bool hasFocus = window.hasFocus();
		if (hasFocus == _imguiSfmlWindowHadFocus) {
			return;
		}

		_imguiSfmlWindowHadFocus = hasFocus;
		if (hasFocus) {
			ImGui::SFML::ProcessEvent(window, sf::Event::FocusGained{});
		}
		else {
			ImGui::SFML::ProcessEvent(window, sf::Event::FocusLost{});
		}
	}

	void MainLoop::Run() {
		sf::Clock mainLoopClock;
		MainContext& mainContext = Engine::MainContext::GetInstance();
		{
			while (true) {
				const auto& window = mainContext.GetMainWindow();
				if (!window) {
					exit(EXIT_FAILURE);
				}
				if (!window->isOpen()) {
					break;
				}

				if (!PollAndDispatchEvents()) {
					break;
				}

				UpdateTick();
				PresentFrame();
			}
		}
	}

	bool MainLoop::PollAndDispatchEvents() {
		auto& mainContext = Engine::MainContext::GetInstance();
		const auto& window = mainContext.GetMainWindow();
		if (!window) {
			return false;
		}

		if (mainContext.IsImGuiInitialized()) {
			SyncImGuiSfmlWindowFocus(*window);
		}

		while (const auto& ev = window->pollEvent()) {
			if (!Verify(ev)) {
				break;
			}
			if (ev->is<sf::Event::Closed>()) {
				window->close();
				return false;
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
		auto& mainContext = Engine::MainContext::GetInstance();
		bool isImGuiInitialized = mainContext.IsImGuiInitialized();
		const auto& window = mainContext.GetMainWindow();

		if (isImGuiInitialized) {
			SyncImGuiSfmlWindowFocus(*window);
			ImGui::SFML::ProcessEvent(*window, event);
			if (event.is<sf::Event::FocusGained>()) {
				_imguiSfmlWindowHadFocus = true;
			}
			else if (event.is<sf::Event::FocusLost>()) {
				_imguiSfmlWindowHadFocus = false;
			}
		}

		if (const auto* resized = event.getIf<sf::Event::Resized>()) {
			mainContext.OnMainWindowResized(resized->size);
		}

		auto& editor = Editor::GetInstance();
		editor.OnEvent(event);

		if (IsImGuiWantCaptureInput(event)) {
			return true;
		}

		bool consumed = editor.GetEditorToolManager().ProcessEvent(event);
		if (!consumed) {
			mainContext.GetEventsDispatcher()->DispatchEvent(event);
		}
		return true;
	}

	void MainLoop::UpdateTick() {
		auto& mainContext = Engine::MainContext::GetInstance();
		mainContext.OnStartUpdateTick();
		auto simulatedDt = mainContext.GetSimTickDt();
		Editor::GetInstance().OnUpdate(simulatedDt);

		if (!mainContext.IsSimPaused()) {
			mainContext.GetPhysicsProcessor()->Update(simulatedDt);
			if (auto scene = mainContext.GetScene()) {
				scene->Update(simulatedDt);
			}
		}
		if (auto scene = mainContext.GetScene()) {
			scene->FlushSceneObjectIndexIfDirty();
		}
	}

	bool MainLoop::PresentFrame() {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto window = mainContext.GetMainWindow();

		if (!window) {
			return false;
		}
		auto scene = mainContext.GetScene();
		if (!scene) {
			return false;
		}

		mainContext.OnStartPresentFrame();
		auto dt = mainContext.GetFrameDt();

		window->clear();
		if (mainContext.IsImGuiInitialized()) {
			SyncImGuiSfmlWindowFocus(*window);
		}
		ImGui::SFML::Update(*window, dt);
		scene->NotifyPresentRec(dt);

		PresentMainWindowScene(*window, scene);

		Engine::Editor::GetInstance().Draw(*window);

		ImGui::SFML::Render(*window);
		window->display();
		return true;
	}

} // namespace Engine
