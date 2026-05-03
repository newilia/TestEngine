#include "EventHandlerBase.h"

#include "EventsDispatcher.h"
#include "MainContext.h"

namespace Engine {
	void EventHandlerBase::SubscribeForEvents() {
		if (_registered) {
			UnsubscribeFromEvents();
		}
		Engine::MainContext::GetInstance().GetEventsDispatcher()->RegisterInputHandler(shared_from_this());
		_registered = true;
	}

	void EventHandlerBase::UnsubscribeFromEvents() {
		if (!_registered) {
			return;
		}
		Engine::MainContext::GetInstance().GetEventsDispatcher()->UnregisterInputHandler(this);
		_registered = false;
	}
} // namespace Engine
