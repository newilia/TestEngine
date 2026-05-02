#pragma once

#include <SFML/Window/Event.hpp>

#include <memory>

namespace Engine {
	/// Registered with the global `UserInput` from `MainContext`; receives every routed window event after
	/// editor/tools (see `MainLoop`). Must be owned by `shared_ptr` before `Register` (`shared_from_this`).
	class InputHandlerBase : public std::enable_shared_from_this<InputHandlerBase>
	{
	public:
		virtual ~InputHandlerBase() = default;

		virtual void OnUserInput(const sf::Event& event) = 0;

		void Register();
		void Unregister();

	private:
		bool _registered = false;
	};
} // namespace Engine
