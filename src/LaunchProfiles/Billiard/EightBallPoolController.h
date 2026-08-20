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
#include "LaunchProfiles/Billiard/OnlineSession.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
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
		void StartHotSeatGame();

		/// @method
		void CreateServerSession();

		/// @method
		void JoinServerSession();

	private:
		void InitSubscriptions();
		void StartNewGame();
		void ResetCue();
		void SpawnBalls();
		void InitPockets();
		void InitScoreboard();
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

		struct MatchLoopConfig
		{
			std::array<PlayerSlotConfig, 2> slots;
			std::array<bool, 2> isLocalAuthorityForRemoteSlot = {true, true};
		};

		void ConfigureMatchLoop(const MatchLoopConfig& config);
		void BindPlayerAgentRuntimeDeps();
		void ConfigureHotSeatMatchLoop();
		void ConfigureOnlineMatchLoop();
		void UpdateAimDisplayInputEnabled();
		void OnBallInHandGrab();
		void OnBallInHandRelease();
		void PollNetworkEvents();
		void BeginOnlineMatch();
		[[nodiscard]] bool IsLocalAuthority() const;
		[[nodiscard]] bool IsPassiveTurn() const;
		void SendCueAimUpdateIfNeeded();
		void SendTableStateUpdateIfNeeded();
		void SendTableStateUpdateToPeer();
		void CheckBallsOutOfBounds();

		/// @property
		AssetRef<SceneObject> _cueAsset;
		/// @property
		RefWrapper<BilliardCueBehaviour> _cueBehaviour;
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
		std::shared_ptr<OnlineSession> _onlineSession;

		bool _isWaitingForBallsToStop = false;
		bool _isDraggingBallInHand = false;
		float _remainingTurnTime = 0.f;
		bool _waitingForGameStart = false;

		EightBallPoolGame _gameState;
		BilliardTablePresenter _tablePresenter;
		BilliardMatchLoop _matchLoop;
		sf::Time _gameTimestamp;
	};

} // namespace Billiard
