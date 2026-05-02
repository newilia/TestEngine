#pragma once

#include "Engine/App/InputHandlerBase.h"

#include <functional>

/// `InputHandlerBase` that forwards to a functor (for environments and glue code).
class FunctionInputHandler final : public Engine::InputHandlerBase
{
public:
	explicit FunctionInputHandler(std::function<void(const sf::Event&)> fn) : _fn(std::move(fn)) {}

	void OnUserInput(const sf::Event& event) override {
		if (_fn) {
			_fn(event);
		}
	}

private:
	std::function<void(const sf::Event&)> _fn;
};
