#pragma once

#include <SFML/Window/Event.hpp>

#include <memory>
#include <vector>

namespace Engine {
	class EventHandlerBase;

	class EventsDispatcher
	{
		friend class EventHandlerBase;

	public:
		void DispatchEvent(const sf::Event& event);

	private:
		void RegisterInputHandler(std::shared_ptr<Engine::EventHandlerBase> handler);
		void UnregisterInputHandler(Engine::EventHandlerBase* handler);

	private:
		std::vector<std::weak_ptr<Engine::EventHandlerBase>> _handlers;
	};
} // namespace Engine
