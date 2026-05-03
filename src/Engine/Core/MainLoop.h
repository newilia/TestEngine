#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

namespace Engine {
	class MainLoop
	{
	public:
		void Run();

	private:
		/// Returns false if the window was closed
		[[nodiscard]] bool PollAndDispatchEvents();
		/* Uses ImGui IO flags from the previous frame(after the last ImGui::SFML::Update), which is
		 the usual pattern for deciding whether application code should see mouse/keyboard events */
		bool IsImGuiWantCaptureInput(const sf::Event& event);
		void UpdateTick();
		bool PresentFrame();
		bool DispatchEvent(const sf::Event& event);
	};
} // namespace Engine
