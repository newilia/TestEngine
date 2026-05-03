#include "InputHandlerBehaviourBase.h"

void InputHandlerBehaviourBase::OnInit() {
	Behaviour::OnInit();
	SubscribeForEvents();
}

void InputHandlerBehaviourBase::OnDeinit() {
	UnsubscribeFromEvents();
	Behaviour::OnDeinit();
}
