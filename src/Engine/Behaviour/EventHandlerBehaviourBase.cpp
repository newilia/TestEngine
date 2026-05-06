#include "EventHandlerBehaviourBase.h"

void EventHandlerBehaviourBase::OnInit() {
	Behaviour::OnInit();
	SubscribeForEvents();
}

void EventHandlerBehaviourBase::OnDeinit() {
	UnsubscribeFromEvents();
	Behaviour::OnDeinit();
}
