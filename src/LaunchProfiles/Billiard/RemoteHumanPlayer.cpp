#include "RemoteHumanPlayer.h"

namespace Billiard {

	RemoteHumanPlayer::RemoteHumanPlayer(
	    int playerIndex, bool isLocalAuthority, std::unique_ptr<LocalHumanPlayer> localDelegate)
	    : _playerIndex(playerIndex), _isLocalAuthority(isLocalAuthority), _localDelegate(std::move(localDelegate)) {}

	void RemoteHumanPlayer::OnTurnStarted(const TableSnapshot& table, const RulesSnapshot& rules) {
		_pendingSnapshot.reset();
		if (_isLocalAuthority && _localDelegate) {
			_localDelegate->OnTurnStarted(table, rules);
		}
	}

	void RemoteHumanPlayer::OnTurnUpdate(const sf::Time& deltaTime) {
		if (_isLocalAuthority && _localDelegate) {
			_localDelegate->OnTurnUpdate(deltaTime);
		}
	}

	void RemoteHumanPlayer::OnTurnEnded() {
		if (_isLocalAuthority && _localDelegate) {
			_localDelegate->OnTurnEnded();
		}
	}

	void RemoteHumanPlayer::OnEvent(const sf::Event& event) {
		if (_isLocalAuthority && _localDelegate) {
			_localDelegate->OnEvent(event);
		}
	}

	bool RemoteHumanPlayer::HasPendingIntent() const {
		if (_isLocalAuthority && _localDelegate) {
			return _localDelegate->HasPendingIntent();
		}
		return false;
	}

	std::optional<TurnIntent> RemoteHumanPlayer::ConsumeIntent() {
		if (_isLocalAuthority && _localDelegate) {
			return _localDelegate->ConsumeIntent();
		}
		return std::nullopt;
	}

	bool RemoteHumanPlayer::WantsInput() const {
		return _isLocalAuthority && _localDelegate && _localDelegate->WantsInput();
	}

	void RemoteHumanPlayer::OnRemoteTurnResult(const TableSnapshot& snapshot) {
		if (!_isLocalAuthority) {
			_pendingSnapshot = snapshot;
		}
	}

	bool RemoteHumanPlayer::HasPendingSnapshot() const {
		return _pendingSnapshot.has_value();
	}

	std::optional<TableSnapshot> RemoteHumanPlayer::ConsumeSnapshot() {
		auto snapshot = _pendingSnapshot;
		_pendingSnapshot.reset();
		return snapshot;
	}

} // namespace Billiard
