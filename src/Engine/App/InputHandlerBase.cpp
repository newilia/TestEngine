#include "InputHandlerBase.h"

#include "MainContext.h"
#include "UserInput.h"

namespace Engine {
	void InputHandlerBase::Register() {
		if (_registered) {
			Unregister();
		}
		Engine::MainContext::GetInstance().GetUserInput()->RegisterInputHandler(shared_from_this());
		_registered = true;
	}

	void InputHandlerBase::Unregister() {
		if (!_registered) {
			return;
		}
		Engine::MainContext::GetInstance().GetUserInput()->UnregisterInputHandler(this);
		_registered = false;
	}
} // namespace Engine
