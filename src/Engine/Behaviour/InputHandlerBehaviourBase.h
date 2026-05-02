#pragma once

#include "Engine/App/InputHandlerBase.h"
#include "Engine/Behaviour/Behaviour.h"

/// Behaviour that registers with `UserInput` during `OnInit` and unregisters in `OnDeinit`.
/// Derived classes should call `InputHandlerBehaviourBase::OnInit()` / `OnDeinit()` when overriding.
class InputHandlerBehaviourBase : public Behaviour, public Engine::InputHandlerBase
{
public:
	void OnInit() override;
	void OnDeinit() override;
};
