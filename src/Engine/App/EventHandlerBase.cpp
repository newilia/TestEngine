#include "EventHandlerBase.h"

#include "EventsDispatcher.h"
#include "MainContext.h"

namespace Engine {
	void EventHandlerBase::SubscribeForInputEvents() {
		if (_registered) {
			UnsubscribeFromInputEvent();
		}
		Engine::MainContext::GetInstance().GetEventsDispatcher()->RegisterInputHandler(shared_from_this());
		_registered = true;
	}

	void EventHandlerBase::UnsubscribeFromInputEvent() {
		if (!_registered) {
			return;
		}
		Engine::MainContext::GetInstance().GetEventsDispatcher()->UnregisterInputHandler(this);
		_registered = false;
	}
} // namespace Engine
