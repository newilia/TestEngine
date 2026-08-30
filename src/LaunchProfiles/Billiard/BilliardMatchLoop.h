#pragma once

#include "BilliardCueBehaviour.h"
#include "BilliardPlayerKind.h"
#include "BilliardTablePresenter.h"
#include "EightBallPoolGame.h"
#include "IPlayerAgent.h"
#include "PlayerAgentFactory.h"

#include <array>
#include <memory>

namespace sf {
	class Event;
}

namespace Billiard {

	class BilliardMatchLoop
	{
	public:
		void Configure(
		    const std::array<PlayerSlotConfig, 2>& slots, const std::array<bool, 2>& isLocalAuthorityForRemoteSlot);
		void BindRuntimeDeps(const std::weak_ptr<BilliardCueBehaviour>& cue, BallInHandInputController* ballInHandInput,
		    CueInputController* cueInput, CueBallAimInputController* cueBallAimInput);
		void OnTurnStarted(EightBallPoolGame& game, BilliardTablePresenter& table);
		void OnTurnEnded();
		void OnUpdate(const sf::Time& deltaTime, EightBallPoolGame& game);
		void OnEvent(const sf::Event& event, EightBallPoolGame& game);
		[[nodiscard]] IPlayerAgent* GetActiveAgent(const EightBallPoolGame& game);
		[[nodiscard]] const std::array<std::unique_ptr<IPlayerAgent>, 2>& GetAgents() const;
		[[nodiscard]] const std::array<PlayerSlotConfig, 2>& GetSlots() const;

	private:
		void ApplyAgentShotIntent(const TurnIntent& intent);

		std::array<PlayerSlotConfig, 2> _slots = {};
		PlayerAgentDeps _deps;
		std::array<std::unique_ptr<IPlayerAgent>, 2> _agents;
	};

} // namespace Billiard
