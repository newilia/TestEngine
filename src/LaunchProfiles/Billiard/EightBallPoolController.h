#pragma once

#include "AimGuideLineBehaviour.h"
#include "BallInHandInputController.h"
#include "BilliardBallAimDisplayBehaviour.h"
#include "BilliardBallBehaviour.h"
#include "BilliardBallSpawnBehaviour.h"
#include "BilliardCueBehaviour.h"
#include "BilliardMatchLoop.h"
#include "BilliardPlayerKind.h"
#include "BilliardPocketBehaviour.h"
#include "BilliardScoreboardBehaviour.h"
#include "BilliardTablePresenter.h"
#include "EightBallPoolGame.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SubscriptionsHolderBase.h"
#include "OnlineSession.h"

#include <array>
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

	public:
		/// @method
		void StartHotSeatGame();
		/// @method
		void CreateServerSession();
		/// @method
		void JoinServerSession();

	private:
		void InitSubscriptions();
		void StartNewGame();
		void InitCue();
		void InitAimGuideLine();
		void SpawnBalls();
		void InitPockets();
		void InitScoreboard();
		void OnBallFellInPocket(int ballNumber, shared_ptr<BilliardPocketBehaviour> pocketRef);
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
			std::array<bool, 2> isLocalAuthorityForRemoteSlot = {true, true}; // todo move to PlayerSlotConfig?
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
		bool IsLocalAuthority() const;
		bool IsPassiveTurn() const;
		void SendCueAimUpdateIfNeeded();
		void SendTableStateUpdateIfNeeded();
		void SendTableStateUpdateToPeer();
		void CheckBallsOutOfTableBounds();
		void UpdateCountdownTimer(float deltaSeconds);
		void OnTurnTimeOver();
		void OnCueAimChanged();
		std::shared_ptr<BilliardBallBehaviour> GetCueBall();

	private:
		/// @property
		RefWrapper<BilliardCueBehaviour> _cueBehaviour;
		/// @property
		RefWrapper<AimGuideLineBehaviour> _aimGuideLineBehaviour;
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
		/// @property
		std::string _serverHost = "130.110.1.72";
		/// @property
		int _serverPort = 7777;

	private:
		std::shared_ptr<OnlineSession> _onlineSession;

		bool _isWaitingForBallsToStop = false;
		bool _isDraggingBallInHand = false;
		float _remainingTurnTime = 0.f;
		bool _waitingForGameStart = false;

		EightBallPoolGame _gameState;
		BilliardTablePresenter _tablePresenter;
		BallInHandInputController _ballInHandInputController{_tablePresenter};
		BilliardMatchLoop _matchLoop;
		sf::Time _gameTimestamp;
	};

} // namespace Billiard
