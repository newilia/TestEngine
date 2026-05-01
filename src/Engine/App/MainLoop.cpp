#include "MainLoop.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Core/PeriodicTaskExecutor.h"
#include "Engine/Editor/Editor.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

// #include <stdlib.h>

namespace Engine {

	void MainLoop::Update() {
		MainContext& engine = Engine::MainContext::GetInstance();
		auto dt = _clock.restart();

		const auto targetTickPeriod = [&engine]() {
			auto period = sf::seconds(1.f / engine.GetTargetTickRate());
			return period;
		};
		PeriodicTaskExecutor tickExecutor(targetTickPeriod, [this](const sf::Time& dt) { UpdateTick(); });

		const auto targetFramePeriod = [&engine]() {
			if (!engine.IsFramerateLimitEnabled()) {
				return sf::Time::Zero;
			}
			auto period = sf::seconds(1.f / engine.GetFramerateLimit());
			return period;
		};
		PeriodicTaskExecutor presentExecutor(targetFramePeriod, [this](const sf::Time& dt) { PresentFrame(); });

		sf::Clock mainLoopClock;

		while (true) {
			auto window = engine.GetMainWindow();
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

	bool MainLoop::PollAndDispatchEvents() {
		auto& engine = Engine::MainContext::GetInstance();
		auto window = engine.GetMainWindow();
		if (!window) {
			return false;
		}

		bool isImGuiInitialized = engine.IsImGuiInitialized();

		while (auto ev = window->pollEvent()) {
			if (isImGuiInitialized) {
				ImGui::SFML::ProcessEvent(*window, *ev);
			}
			sf::Event event = *ev;
			if (event.is<sf::Event::Closed>()) {
				window->close();
				return false;
			}
			if (const auto* resized = event.getIf<sf::Event::Resized>()) {
				const auto sz = resized->size;
				// TODO fix camera reset
				window->setView(
				    sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(sz.x), static_cast<float>(sz.y)})));
			}
			Editor::GetInstance().OnEvent(event);
			const bool forwardToGame = !isImGuiInitialized || ShouldForwardEventToGame(event);
			bool consumed = false;
			if (forwardToGame) {
				consumed = Editor::GetInstance().GetEditorToolManager().ProcessEvent(event);
			}
			if (forwardToGame && !consumed) {
				engine.GetUserInput()->HandleEvent(event);
			}
		}
		return window->isOpen();
	}

	bool MainLoop::ShouldForwardEventToGame(const sf::Event& event) {
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

	bool MainLoop::UpdateTick() {
		auto& engine = Engine::MainContext::GetInstance();
		if (engine.IsSimPaused()) {
			return false;
		}
		engine.OnStartUpdateTick();
		auto simulatedDt = engine.GetSimTickDt();

		if (auto scene = engine.GetScene()) {
			scene->UpdateRec(simulatedDt);
		}
		engine.GetPhysicsProcessor()->Update(simulatedDt);
		return true;
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
