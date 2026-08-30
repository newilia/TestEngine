#pragma once

#include "BallInHandInputController.h"
#include "BilliardTurnIntent.h"
#include "CueBallAimInputController.h"
#include "CueInputController.h"
#include "IPlayerAgent.h"

#include <cstdint>
#include <optional>

namespace Billiard {

	class LocalHumanPlayer : public IPlayerAgent
	{
	public:
		LocalHumanPlayer(int playerIndex, BallInHandInputController* ballInHandInput, CueInputController* cueInput,
		    CueBallAimInputController* cueBallAimInput);

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
		BallInHandInputController* _ballInHandInput = nullptr;
		CueInputController* _cueInput = nullptr;
		CueBallAimInputController* _cueBallAimInput = nullptr;
		bool _inputEnabled = false;
	};

} // namespace Billiard
