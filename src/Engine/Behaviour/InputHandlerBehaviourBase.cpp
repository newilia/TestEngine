#include "InputHandlerBehaviourBase.h"

void InputHandlerBehaviourBase::OnInit() {
	Behaviour::OnInit();
	Register();
}

void InputHandlerBehaviourBase::OnDeinit() {
	Unregister();
	Behaviour::OnDeinit();
}
