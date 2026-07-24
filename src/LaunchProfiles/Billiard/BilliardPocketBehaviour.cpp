#include "BilliardPocketBehaviour.h"

#include "BilliardPocketBehaviour.generated.hpp"

namespace Billiard {

	void BilliardPocketBehaviour::RegisterBallFall(BilliardBallBehaviour& ball) {
		RefWrapper<BilliardBallBehaviour> ballRef;
		ballRef.SetId(ball.GetEntityId());
		_ballsInPocket.push_back(ballRef);
		_onBallFall.Emit(ball.GetBallNumber());
	}

	Signal<int>& BilliardPocketBehaviour::GetOnBallFallSignal() const {
		return _onBallFall;
	}

} // namespace Billiard
