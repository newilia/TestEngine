#include "BilliardScoreboardBehaviour.h"

#include "BilliardScoreboardBehaviour.generated.hpp"

#include <algorithm>

namespace Billiard {

	void BilliardScoreboardBehaviour::OnUpdate(const sf::Time& dt) {
		UpdateTimer(dt);
	}

	void BilliardScoreboardBehaviour::Reset() {
		_timerSeconds = 0.f;
		_activePlayerIndex = 0;
	}

	void BilliardScoreboardBehaviour::ShowMessage(const std::string& message) {
		_message = message;
	}

	void BilliardScoreboardBehaviour::SetActivePlayerIndex(int playerIndex) {
		_activePlayerIndex = playerIndex;
	}

	void BilliardScoreboardBehaviour::UpdateTimer(const sf::Time& dt) {
		if (_timerSeconds > 0.f) {
			_timerSeconds = std::max(0.f, _timerSeconds - dt.asSeconds());
		}
	}

} // namespace Billiard
