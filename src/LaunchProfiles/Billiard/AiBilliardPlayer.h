#pragma once

#include "BilliardCueBehaviour.h"
#include "BilliardPlayerKind.h"
#include "BilliardTurnIntent.h"
#include "IPlayerAgent.h"

#include <memory>
#include <optional>

namespace Billiard {

	class AiBilliardPlayer : public IPlayerAgent
	{
	public:
		explicit AiBilliardPlayer(int playerIndex);

		void OnTurnStarted(const TableSnapshot& table, const RulesSnapshot& rules) override;
		void OnTurnUpdate(const sf::Time& deltaTime) override;
		void OnTurnEnded() override;
		void OnEvent(const sf::Event& event) override;
		[[nodiscard]] bool HasPendingIntent() const override;
		std::optional<TurnIntent> ConsumeIntent() override;
		[[nodiscard]] bool WantsInput() const override;

	private:
		int _playerIndex = 0;
		std::uint32_t _turnId = 0;
		float _thinkTimeRemaining = 0.f;
		bool _hasShotThisTurn = false;
		std::optional<TurnIntent> _pendingIntent;
	};

} // namespace Billiard
