#pragma once

#include <SFML/Window/Event.hpp>

#include <memory>

namespace Engine {
	/// Registered with the global `EventsDispatcher` from `MainContext`; receives every routed window event after
	/// editor/tools (see `MainLoop`). Must be owned by `shared_ptr` before `Register` (`shared_from_this`).
	class EventHandlerBase : public std::enable_shared_from_this<EventHandlerBase>
	{
	public:
		virtual ~EventHandlerBase() = default;
		virtual void OnEvent(const sf::Event& event) = 0;

		void SubscribeForEvents();
		void UnsubscribeFromEvents();

	private:
		bool _registered = false;
	};
} // namespace Engine
