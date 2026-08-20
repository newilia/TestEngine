#include "BilliardScoreboardBehaviour.h"

#include "BilliardScoreboardBehaviour.generated.hpp"
#include "EightBallPoolGame.h"
#include "Engine/Visual/TextVisual.h"

namespace Billiard {

	void BilliardScoreboardBehaviour::Reset() {
		SetRemainingTurnTime(-1.f);
		SetActivePlayerIndex(0);
		SetPlayerBallType(0, BallType::Undefined);
		SetPlayerBallType(1, BallType::Undefined);
		ShowMessage("");
	}

	void BilliardScoreboardBehaviour::SetPlayerName(int playerIndex, const std::string& name) {
		auto textRef = playerIndex == 0 ? _player1nameTextRef : _player2nameTextRef;
		if (auto text = textRef.Get()) {
			text->SetString(name);
		}
	}

	void BilliardScoreboardBehaviour::ShowMessage(const std::string& message) {
		if (auto messageText = _messageTextRef.Get()) {
			messageText->SetString(message);
		}
	}

	void BilliardScoreboardBehaviour::ShowFoulMessage(FoulKind foulKind) {
		switch (foulKind) {
		case FoulKind::None:
			ShowMessage("");
			break;
		case FoulKind::CueBallPocketed:
			ShowMessage("Foul! Cue ball pocketed. Ball in hand");
			break;
		case FoulKind::WrongBallFirst:
			ShowMessage("Foul! Wrong ball first. Ball in hand");
			break;
		case FoulKind::BreakInsufficientRails:
			ShowMessage("Foul! Break - at least 4 balls must collide with rails. Ball in hand");
			break;
		case FoulKind::NoRailContact:
			ShowMessage("Foul! No balls collided with rails. Ball in hand");
			break;
		case FoulKind::TurnTimeOver:
			ShowMessage("Foul! Turn time expired. Ball in hand");
			break;
		default:
			ShowMessage("Foul! Ball in hand");
			break;
		}
	}

	void BilliardScoreboardBehaviour::SetActivePlayerIndex(int playerIndex) {
		if (auto player1activeNode = _player1activeNodeRef.Get()) {
			player1activeNode->SetVisible(playerIndex == 0);
		}
		if (auto player2activeNode = _player2activeNodeRef.Get()) {
			player2activeNode->SetVisible(playerIndex == 1);
		}
	}

	void BilliardScoreboardBehaviour::SetRemainingTurnTime(float seconds) {
		if (auto timerText = _timerTextRef.Get()) {
			if (seconds < 0.f) {
				timerText->SetString("");
			}
			else {
				timerText->SetString(fmt::format("{}s", static_cast<int>(seconds)));
			}
		}
	}

	void BilliardScoreboardBehaviour::SetPlayerBallType(int playerIndex, BallType ballType) {
		auto textRef = playerIndex == 0 ? _player1ballTypeTextRef : _player2ballTypeTextRef;
		if (auto text = textRef.Get()) {
			std::string str;
			switch (ballType) {
			case BallType::Striped:
				str = "Striped";
				break;
			case BallType::Solid:
				str = "Solid";
				break;
			}
			text->SetString(str);
		}
	}
} // namespace Billiard
