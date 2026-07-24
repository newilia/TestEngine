#include "BilliardScoreboardBehaviour.h"

#include "BilliardScoreboardBehaviour.generated.hpp"

#include <algorithm>

namespace Billiard {

	void BilliardScoreboardBehaviour::OnUpdate(const sf::Time& dt) {
		UpdateTimer(dt);
	}

	void BilliardScoreboardBehaviour::SetPlayerScore(int playerIndex, int score) {
		if (playerIndex == 0) {
			_player1Score = score;
		}
		else if (playerIndex == 1) {
			_player2Score = score;
		}
	}

	void BilliardScoreboardBehaviour::AddPlayerScore(int playerIndex, int delta) {
		if (playerIndex == 0) {
			_player1Score += delta;
		}
		else if (playerIndex == 1) {
			_player2Score += delta;
		}
	}

	void BilliardScoreboardBehaviour::ShowMessage(const std::string& message) {
		_message = message;
	}

	void BilliardScoreboardBehaviour::SetTimer(float seconds) {
		_timerSeconds = seconds;
	}

	void BilliardScoreboardBehaviour::SetActivePlayer(int playerIndex) {
		_activePlayerIndex = playerIndex;
	}

	void BilliardScoreboardBehaviour::UpdateTimer(const sf::Time& dt) {
		if (_timerSeconds > 0.f) {
			_timerSeconds = std::max(0.f, _timerSeconds - dt.asSeconds());
		}
	}

} // namespace Billiard
