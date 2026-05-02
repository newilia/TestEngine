#include "InputHandlerBehaviourBase.h"

void InputHandlerBehaviourBase::OnInit() {
	Behaviour::OnInit();
	SubscribeForInputEvents();
}

void InputHandlerBehaviourBase::OnDeinit() {
	UnsubscribeFromInputEvent();
	Behaviour::OnDeinit();
}
