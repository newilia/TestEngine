#pragma once

#include "BallInHandInputController.h"
#include "BilliardBallBehaviour.h"
#include "BilliardCueBehaviour.h"
#include "BilliardPlayerKind.h"
#include "CueBallAimInputController.h"
#include "CueInputController.h"
#include "IPlayerAgent.h"

#include <array>
#include <memory>

namespace Billiard {

	struct PlayerAgentDeps

	{
		std::weak_ptr<BilliardCueBehaviour> cue;

		BallInHandInputController* ballInHandInput = nullptr;

		CueInputController* cueInput = nullptr;

		CueBallAimInputController* cueBallAimInput = nullptr;

		std::array<bool, 2> isLocalAuthorityForRemoteSlot = {true, true};
	};

	std::array<std::unique_ptr<IPlayerAgent>, 2> CreatePlayerAgents(

	    const std::array<PlayerSlotConfig, 2>& slots, const PlayerAgentDeps& deps);

} // namespace Billiard
