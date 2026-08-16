#pragma once

#include "BilliardTableSnapshot.h"
#include "BilliardTurnIntent.h"
#include "IPlayerAgent.h"
#include "LocalHumanPlayer.h"

#include <memory>
#include <optional>

namespace Billiard {

	class OnlineSessionBehaviour;

	class RemoteHumanPlayer : public IPlayerAgent
	{
	public:
		RemoteHumanPlayer(int playerIndex, bool isLocalAuthority, std::unique_ptr<LocalHumanPlayer> localDelegate);

		void OnTurnStarted(const TableSnapshot& table, const RulesSnapshot& rules) override;
		void OnTurnUpdate(const sf::Time& deltaTime) override;
		void OnTurnEnded() override;
		void OnEvent(const sf::Event& event) override;
		[[nodiscard]] bool HasPendingIntent() const override;
		std::optional<TurnIntent> ConsumeIntent() override;
		[[nodiscard]] bool WantsInput() const override;

		void OnRemoteTurnResult(const TableSnapshot& snapshot);
		[[nodiscard]] bool HasPendingSnapshot() const;
		std::optional<TableSnapshot> ConsumeSnapshot();

	private:
		int _playerIndex = 0;
		bool _isLocalAuthority = false;
		std::unique_ptr<LocalHumanPlayer> _localDelegate;
		std::optional<TableSnapshot> _pendingSnapshot;
	};

} // namespace Billiard
