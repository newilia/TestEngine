#pragma once

#include <SFML/Window/Event.hpp>

#include <memory>
#include <vector>

namespace Engine {
	class InputHandlerBase;

	class UserInput
	{
		friend class InputHandlerBase;

	public:
		void HandleEvent(const sf::Event& event);

	private:
		void RegisterInputHandler(std::shared_ptr<Engine::InputHandlerBase> handler);
		void UnregisterInputHandler(Engine::InputHandlerBase* handler);

	private:
		std::vector<std::shared_ptr<Engine::InputHandlerBase>> _handlers; // TODO fix memory leak, use weak_ptr
	};
} // namespace Engine
