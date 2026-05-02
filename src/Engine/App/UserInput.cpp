#include "UserInput.h"

#include "InputHandlerBase.h"

#include <algorithm>

namespace Engine {
	void UserInput::HandleEvent(const sf::Event& event) {
		for (auto& h : _handlers) {
			h->OnUserInput(event);
		}
	}

	void UserInput::RegisterInputHandler(std::shared_ptr<InputHandlerBase> handler) {
		if (!Verify(handler)) {
			return;
		}
		for (const auto& existing : _handlers) {
			if (existing == handler) {
				return;
			}
		}
		_handlers.push_back(std::move(handler));
	}

	void UserInput::UnregisterInputHandler(InputHandlerBase* handler) {
		if (!handler) {
			return;
		}
		std::erase_if(_handlers, [handler](const std::shared_ptr<InputHandlerBase>& p) {
			return p.get() == handler;
		});
	}
} // namespace Engine
