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
#include "LaunchProfiles/Billiard/EightBallPoolGame.h"

#include <map>
#include <vector>

namespace Billiard {
	class EightBallPoolController : public Behaviour, public SubscriptionsHolderBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnDeinit() override;
		void OnUpdate(const sf::Time& deltaTime) override;

		/// @method
		void StartNewGame();

	private:
		void InitSubscriptions();
		void SpawnCue();
		void SetCueOnBall(int ballNumber);
		void OnBallFellInPocket(int ballNumber);
		void OnAimPointChanged(const sf::Vector2f& aimPoint);
		bool AreBallsMoving() const;
		void OnCueRelease();
		void OnCueHit();
		void OnBallsStopped();
		void RestoreBall(int ballNumber);
		void UpdateScoreboard();
		void UpdateScoreboardTimer();
		void OnBallCollision(
		    std::shared_ptr<PhysicsBodyBehaviour> ballBody, int ballIndex, const IntersectionDetails& intersection);
		void StartNewTurn();

		float GetBallRadius() const;
		sf::FloatRect GetBallInHandRect() const;
		sf::FloatRect GetKitchenRect() const;
		sf::Vector2f GetTableCenter() const;
		void OnBallInHandGrab();
		void OnBallInHandRelease();

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
		RefWrapper<RectangleShapeVisual> _tableRect;
		/// @property
		float _turnTimeLimit = 30.f;

	private:
		bool _isWaitingForBallsToStop = false;
		float _remainingTurnTime = 0.f;

		EightBallPoolGame _gameState;
		sf::Time _gameTimestamp;
	};

} // namespace Billiard
