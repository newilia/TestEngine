#pragma once

#include "BilliardCueBehaviour.h"
#include "BilliardTurnIntent.h"
#include "IPlayerAgent.h"

#include <memory>
#include <optional>

namespace Billiard {

	class LocalHumanPlayer : public IPlayerAgent
	{
	public:
		LocalHumanPlayer(
		    int playerIndex, std::weak_ptr<BilliardCueBehaviour> cue, std::weak_ptr<BilliardBallBehaviour> cueBall);

		void OnTurnStarted(const TableSnapshot& table, const RulesSnapshot& rules) override;
		void OnTurnUpdate(const sf::Time& deltaTime) override;
		void OnTurnEnded() override;
		void OnEvent(const sf::Event& event) override;
		[[nodiscard]] bool HasPendingIntent() const override;
		std::optional<TurnIntent> ConsumeIntent() override;
		[[nodiscard]] bool WantsInput() const override;

	private:
		[[nodiscard]] sf::Vector2f MapPixelToWorld(sf::Vector2i pixel) const;

		int _playerIndex = 0;
		std::uint32_t _turnId = 0;
		std::weak_ptr<BilliardCueBehaviour> _cue;
		std::weak_ptr<BilliardBallBehaviour> _cueBall;
		bool _inputEnabled = false;
		std::optional<TurnIntent> _pendingIntent;
	};

} // namespace Billiard
