#pragma once

#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SubscriptionsHolderBase.h"
#include "LaunchProfiles/Billiard/BilliardBallAimDisplayBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardBallBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardBallSpawnBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardCueBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardMatchLoop.h"
#include "LaunchProfiles/Billiard/BilliardPlayerKind.h"
#include "LaunchProfiles/Billiard/BilliardPocketBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardScoreboardBehaviour.h"
#include "LaunchProfiles/Billiard/BilliardTablePresenter.h"
#include "LaunchProfiles/Billiard/EightBallPoolGame.h"
#include "LaunchProfiles/Billiard/OnlineSessionBehaviour.h"

#include <array>
#include <cstdint>
#include <map>
#include <vector>

namespace Billiard {
	class EightBallPoolController : public EventHandlerBehaviourBase, public SubscriptionsHolderBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnDeinit() override;
		void OnUpdate(const sf::Time& deltaTime) override;
		void OnEvent(const sf::Event& event) override;

		/// @method
		void StartNewGame();

		/// @method
		void CreateServerSession();

		/// @method
		void JoinServerSession();

	private:
		void InitSubscriptions();
		void SpawnCue();
		void SetCueOnBall(int ballNumber);
		void OnBallFellInPocket(int ballNumber);
		void OnAimPointChanged(const sf::Vector2f& aimPoint);
		void OnCueRelease();
		void OnCueHit();
		void OnBallsStopped();
		void ApplyTurnResolutionUI();
		void UpdateScoreboard();
		void UpdateScoreboardTimer();
		void OnBallCollision(
		    std::shared_ptr<PhysicsBodyBehaviour> ballBody, int ballIndex, const IntersectionDetails& intersection);
		void StartNewTurn();
		void ConfigureMatchLoop();
		void ConfigureOnlineMatchLoop();
		void UpdateAimDisplayInputEnabled();
		void OnBallInHandGrab();
		void OnBallInHandRelease();
		void PollNetworkEvents();
		void BeginOnlineMatch();
		[[nodiscard]] bool IsLocalAuthority() const;
		void UpdatePassiveTurnState();
		void SendCueAimUpdateIfNeeded();
		void SendTableStateUpdateIfNeeded();

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
		RefWrapper<OnlineSessionBehaviour> _onlineSession;
		/// @property
		float _turnTimeLimit = 30.f;

	private:
		PlayerKind _player0Kind = PlayerKind::LocalHuman;
		PlayerKind _player1Kind = PlayerKind::LocalHuman;
		bool _player0IsLocalAuthority = true;
		bool _player1IsLocalAuthority = true;
		std::string _player0Name = "Player 1";
		std::string _player1Name = "Player 2";
		bool _isWaitingForBallsToStop = false;
		float _remainingTurnTime = 0.f;
		bool _isOnlineMatch = false;
		bool _waitingForGameStart = false;
		int _localPlayerIndex = 0;
		std::uint32_t _networkTurnId = 0;
		bool _isPassiveTurn = false;
		int _shootingPlayerIndex = -1;
		float _aimSendAccumulator = 0.f;
		float _tableSendAccumulator = 0.f;

		EightBallPoolGame _gameState;
		BilliardTablePresenter _tablePresenter;
		BilliardMatchLoop _matchLoop;
		sf::Time _gameTimestamp;
	};

} // namespace Billiard
