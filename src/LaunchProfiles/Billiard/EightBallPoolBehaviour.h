#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SubscriptionsHolderBase.h"
#include "LaunchProfiles/Billiard/BilliardBallAimDisplayBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardBallBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardBallSpawnBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardCueBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardPocketBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardScoreboardBehaviour.h"

#include <vector>

namespace Billiard {

	META_ENUM(GamePhase, Break, OpenTable, Solids, Stripes, EightBall, GameOver);

	class EightBallPoolBehaviour : public Behaviour, public SubscriptionsHolderBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnDeinit() override;

		/// @method
		void StartNewGame();

	private:
		void WirePocketSignals();
		void OnBallFellInPocket(int ballNumber);
		void StartTurn(int playerIndex);
		void EndTurn();

		/// @property
		RefWrapper<BilliardCueBehaviour> _cue;
		/// @property
		RefWrapper<BilliardScoreboardBehaviour> _scoreboard;
		/// @property
		RefWrapper<BilliardBallAimDisplayBehaviour> _aimDisplay;
		/// @property
		RefWrapper<BilliardBallSpawnBehaviour> _ballSpawn;
		/// @property
		std::vector<RefWrapper<BilliardPocketBehaviour>> _pockets;
		/// @property
		std::vector<RefWrapper<BilliardBallBehaviour>> _balls;

		/// @property
		GamePhase _phase = GamePhase::Break;
		int _activePlayerIndex = 0;
	};

} // namespace Billiard
