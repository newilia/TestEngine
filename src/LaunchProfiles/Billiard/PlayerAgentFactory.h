#pragma once

#include "BilliardBallBehaviour.h"
#include "BilliardCueBehaviour.h"
#include "BilliardPlayerKind.h"
#include "IPlayerAgent.h"

#include <array>
#include <memory>

namespace Billiard {

	struct PlayerAgentDeps
	{
		std::weak_ptr<BilliardCueBehaviour> cue;
		std::weak_ptr<BilliardBallBehaviour> cueBall;
		std::array<bool, 2> isLocalAuthorityForRemoteSlot = {true, true};
	};

	std::array<std::unique_ptr<IPlayerAgent>, 2> CreatePlayerAgents(
	    const std::array<PlayerSlotConfig, 2>& slots, const PlayerAgentDeps& deps);

} // namespace Billiard
