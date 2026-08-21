#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "LaunchProfiles/Billiard/BilliardBallBehaviour.h"

#include <vector>

namespace Billiard {

	class BilliardPocketBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

	public:
		void Reset();
		void RegisterBall(const BilliardBallBehaviour& ball);
		void PocketBall(BilliardBallBehaviour& ballBehaviour);
		void UnpocketBall(int ballNumber);
		Signal<int>& GetOnBallFallSignal() const;

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _pocketShape;
		/// @property
		std::vector<RefWrapper<BilliardBallBehaviour>> _balls;

	private:
		mutable Signal<int> _onBallFallSignal;
		int _nextBallCollisionGroup = 1;
		std::set<int> _fallenBalls;
	};

} // namespace Billiard
