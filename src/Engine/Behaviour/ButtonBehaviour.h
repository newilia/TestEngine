#pragma once

#include "Engine/Behaviour/InputHandlerBehaviourBase.h"
#include "Engine/Core/Signal.h"

#include <SFML/Window/Event.hpp>

/// Registers for window input and emits **signals** when the pointer taps / releases over this node's `Visual`
/// (see `Visual::HitTest`). Mouse: left button. Touch: first finger (`finger == 0`) only.
class ButtonBehaviour : public InputHandlerBehaviourBase
{
public:
	void OnEvent(const sf::Event& event) override;

	template <class F>
	    requires std::is_invocable_v<F, const sf::Event&>
	[[nodiscard]] Signal<const sf::Event&>::Connection SubscribeOnTap(F&& callable) {
		return _onTap.Connect(std::forward<F>(callable));
	}

	template <class F>
	    requires std::is_invocable_v<F, const sf::Event&>
	[[nodiscard]] Signal<const sf::Event&>::Connection SubscribeOnRelease(F&& callable) {
		return _onRelease.Connect(std::forward<F>(callable));
	}

private:
	[[nodiscard]] bool HitTestWorld(const sf::Vector2f& worldPoint) const;

	Signal<const sf::Event&> _onTap;
	Signal<const sf::Event&> _onRelease;

	bool _mouseDown = false;
	bool _touchDown = false;
	unsigned int _touchFinger = 0;
};
