#include "EventsDispatcher.h"

#include "EventHandlerBase.h"

#include <algorithm>

namespace Engine {
	void EventsDispatcher::DispatchEvent(const sf::Event& event) {
		for (auto it = _handlers.begin(); it != _handlers.end();) {
			if (auto handler = it->lock()) {
				handler->OnEvent(event);
				++it;
			}
			else {
				it = _handlers.erase(it);
			}
		}
	}

	void EventsDispatcher::RegisterInputHandler(std::shared_ptr<EventHandlerBase> handler) {
		if (!Verify(handler)) {
			return;
		}
		for (const auto& existing : _handlers) {
			if (auto e = existing.lock()) {
				if (VerifyFalse(e == handler)) {
					return;
				}
			}
		}
		_handlers.push_back(std::move(handler));
	}

	void EventsDispatcher::UnregisterInputHandler(EventHandlerBase* handler) {
		if (!handler) {
			return;
		}
		std::erase_if(_handlers, [handler](const std::weak_ptr<EventHandlerBase>& p) {
			if (auto sp = p.lock()) {
				return sp.get() == handler;
			}
			return false;
		});
	}
} // namespace Engine
