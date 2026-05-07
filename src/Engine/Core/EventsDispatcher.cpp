#include "EventsDispatcher.h"

#include "EventHandlerBase.h"

#include <algorithm>

namespace Engine {
	void EventsDispatcher::DispatchEvent(const sf::Event& event) {
		/* TODO fix potential handler skip when the handler before unregisters while handling */
		for (int i = 0; i < _handlers.size(); ++i) {
			if (auto handler = _handlers[i].lock()) {
				handler->OnEvent(event);
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
		for (size_t i = 0; i < _handlers.size(); ++i) {
			if (auto sp = _handlers[i].lock()) {
				if (sp.get() == handler) {
					_handlers.erase(_handlers.begin() + i);
					break;
				}
			}
		}
	}
} // namespace Engine
