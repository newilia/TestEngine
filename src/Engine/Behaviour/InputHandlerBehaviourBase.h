#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EventHandlerBase.h"

/// Behaviour that registers with `EventsDispatcher` during `OnInit` and unregisters in `OnDeinit`.
/// Derived classes should call `InputHandlerBehaviourBase::OnInit()` / `OnDeinit()` when overriding.
class InputHandlerBehaviourBase : public Behaviour, public Engine::EventHandlerBase
{
public:
	void OnInit() override;
	void OnDeinit() override;
};
