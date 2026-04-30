#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

namespace Engine {
	class MainLoop
	{
	public:
		void Update();

	private:
		/// Returns false if the window was closed. ImGui-SFML APIs require Init first (`imguiSfmlReady`).
		[[nodiscard]] bool PollAndDispatchEvents();

		// Uses ImGui IO flags from the previous frame (after the last ImGui::SFML::Update), which is
		// the usual pattern for deciding whether application code should see mouse/keyboard events.
		[[nodiscard]] bool ShouldForwardEventToGame(const sf::Event& event);

		bool UpdateTick();

		bool PresentFrame();

	private:
		sf::Clock _clock;
	};
} // namespace Engine
