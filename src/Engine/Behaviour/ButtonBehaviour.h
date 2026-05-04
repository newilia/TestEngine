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

	Signal<>& GetOnTapSignal() const;
	Signal<>& GetOnReleaseSignal() const;

private:
	bool HitTestWorld(const sf::Vector2f& worldPoint) const;

	mutable Signal<> _onTap;
	mutable Signal<> _onRelease;

	bool _mouseDown = false;
	bool _touchDown = false;
	unsigned int _touchFinger = 0;
};
