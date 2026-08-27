#include "BilliardMatchLoop.h"

namespace Billiard {

	void BilliardMatchLoop::Configure(
	    const std::array<PlayerSlotConfig, 2>& slots, const std::array<bool, 2>& isLocalAuthorityForRemoteSlot) {
		_slots = slots;
		_deps.isLocalAuthorityForRemoteSlot = isLocalAuthorityForRemoteSlot;
	}

	void BilliardMatchLoop::BindRuntimeDeps(
	    const std::weak_ptr<BilliardCueBehaviour>& cue, const std::weak_ptr<BilliardBallBehaviour>& cueBall) {
		_deps.cue = cue;
		_deps.cueBall = cueBall;
		_agents = CreatePlayerAgents(_slots, _deps);
	}

	void BilliardMatchLoop::OnTurnStarted(EightBallPoolGame& game, BilliardTablePresenter& table) {
		const auto tableSnapshot = table.CaptureSnapshot();
		const auto rulesSnapshot = game.ToSnapshot();
		if (auto* agent = GetActiveAgent(game)) {
			agent->OnTurnStarted(tableSnapshot, rulesSnapshot);
		}
	}

	void BilliardMatchLoop::OnTurnEnded() {
		for (auto& agent : _agents) {
			if (agent) {
				agent->OnTurnEnded();
			}
		}
	}

	void BilliardMatchLoop::OnUpdate(const sf::Time& deltaTime, EightBallPoolGame& game) {
		if (game.IsGameOver()) {
			return;
		}
		auto* agent = GetActiveAgent(game);
		if (!agent) {
			return;
		}
		agent->OnTurnUpdate(deltaTime);

		if (!agent->HasPendingIntent()) {
			return;
		}

		const int activeIndex = game.GetActivePlayerIndex();
		const auto kind = _slots[static_cast<std::size_t>(activeIndex)].kind;
		if (kind == PlayerKind::LocalHuman || kind == PlayerKind::RemoteHuman) {
			agent->ConsumeIntent();
			return;
		}

		if (auto intent = agent->ConsumeIntent()) {
			if (intent->phase == TurnIntentPhase::Shoot) {
				ApplyAgentShotIntent(*intent);
			}
		}
	}

	void BilliardMatchLoop::OnEvent(const sf::Event& event, EightBallPoolGame& game) {
		if (auto* agent = GetActiveAgent(game)) {
			if (agent->WantsInput()) {
				agent->OnEvent(event);
			}
		}
	}

	IPlayerAgent* BilliardMatchLoop::GetActiveAgent(const EightBallPoolGame& game) {
		const int index = game.GetActivePlayerIndex();
		if (index < 0 || index >= 2) {
			return nullptr;
		}
		return _agents[static_cast<std::size_t>(index)].get();
	}

	const std::array<std::unique_ptr<IPlayerAgent>, 2>& BilliardMatchLoop::GetAgents() const {
		return _agents;
	}

	const std::array<PlayerSlotConfig, 2>& BilliardMatchLoop::GetSlots() const {
		return _slots;
	}

	void BilliardMatchLoop::ApplyAgentShotIntent(const TurnIntent& intent) {
		if (auto cue = _deps.cue.lock()) {
			cue->SetInputEnabled(true);
			cue->ApplyShotIntent(intent);
		}
	}

} // namespace Billiard
