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

#include <map>
#include <vector>

namespace Billiard {

	META_ENUM(GamePhase, Break, OpenTable, Solids, Stripes, EightBall, GameOver);

	class EightBallPoolBehaviour : public Behaviour, public SubscriptionsHolderBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnDeinit() override;
		void OnUpdate(const sf::Time& deltaTime) override;

		/// @method
		void StartNewGame();

	private:
		void SpawnCue();
		void SetCueOnBall(int ballNumber);
		void OnBallFellInPocket(int ballNumber);
		void StartTurn(int playerIndex);
		void EndTurn();
		void OnAimPointChanged(const sf::Vector2f& aimPoint);
		bool AreBallsMoving() const;
		void OnCueHit();
		void OnBallsStopped();
		void RestoreCueBall();

		/// @property
		AssetRef<SceneObject> _cueAsset;
		/// @property
		RefWrapper<BilliardCueBehaviour> _cueBehaviour;
		/// @property
		RefWrapper<SceneNode> _cueParent;
		/// @property
		RefWrapper<BilliardScoreboardBehaviour> _scoreboardBehaviour;
		/// @property
		RefWrapper<BilliardBallAimDisplayBehaviour> _aimDisplayBehaviour;
		/// @property
		RefWrapper<BilliardBallSpawnBehaviour> _ballSpawnBehaviour;
		/// @property
		std::vector<RefWrapper<BilliardPocketBehaviour>> _pocketsBehaviours;
		/// @property
		std::map<int, RefWrapper<BilliardBallBehaviour>> _ballsBehaviours;
		/// @property
		GamePhase _phase = GamePhase::Break;
		/// @property
		int _activePlayerIndex = 0;

	private:
		bool _isWaitingForBallsToStop = false;
		std::set<int> _pocketedBalls;
		//Signal<sf::Vector2f>::Subscription _aimPointChangedSubscription;
		//Signal<>::Subscription _onCueReleaseSubscription;
		//Signal<int>::Subscription _onBallFellInPocketSubscription;
	};

} // namespace Billiard
