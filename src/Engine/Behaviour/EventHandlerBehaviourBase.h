#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EventHandlerBase.h"

/// Behaviour that registers with `EventsDispatcher` during `OnInit` and unregisters in `OnDeinit`.
/// Derived classes should call `EventHandlerBehaviourBase::OnInit()` / `OnDeinit()` when overriding.
class EventHandlerBehaviourBase : public Behaviour, public Engine::EventHandlerBase
{
public:
	void OnInit() override;
	void OnDeinit() override;
};
