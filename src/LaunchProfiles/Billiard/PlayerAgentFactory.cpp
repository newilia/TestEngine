#include "PlayerAgentFactory.h"

#include "AiBilliardPlayer.h"
#include "LocalHumanPlayer.h"
#include "RemoteHumanPlayer.h"

namespace Billiard {

	std::array<std::unique_ptr<IPlayerAgent>, 2> CreatePlayerAgents(
	    const std::array<PlayerSlotConfig, 2>& slots, const PlayerAgentDeps& deps) {
		std::array<std::unique_ptr<IPlayerAgent>, 2> agents;
		for (int playerIndex = 0; playerIndex < 2; ++playerIndex) {
			switch (slots[static_cast<std::size_t>(playerIndex)].kind) {
			case PlayerKind::LocalHuman:
				agents[static_cast<std::size_t>(playerIndex)] =
				    std::make_unique<LocalHumanPlayer>(playerIndex, deps.cue, deps.cueBall, deps.ballInHandInput);
				break;
			case PlayerKind::RemoteHuman: {
				auto localDelegate =
				    std::make_unique<LocalHumanPlayer>(playerIndex, deps.cue, deps.cueBall, deps.ballInHandInput);
				const bool isLocalAuthority = deps.isLocalAuthorityForRemoteSlot[static_cast<std::size_t>(playerIndex)];
				agents[static_cast<std::size_t>(playerIndex)] =
				    std::make_unique<RemoteHumanPlayer>(playerIndex, isLocalAuthority, std::move(localDelegate));
				break;
			}
			case PlayerKind::Ai:
				agents[static_cast<std::size_t>(playerIndex)] = std::make_unique<AiBilliardPlayer>(playerIndex);
				break;
			}
		}
		return agents;
	}

} // namespace Billiard
