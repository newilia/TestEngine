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
		Signal<int>& GetOnBallPocketedSignal() const;
		int UseBallCollisionGroup();
		shared_ptr<CircleShapeVisual> GetPocketShape() const;

	private:
		void OnBallPocketed(BilliardBallBehaviour& ballBehaviour);

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _pocketShape;

	private:
		std::vector<RefWrapper<BilliardBallBehaviour>> _balls;
		mutable Signal<int> _onBallPocketedSignal;
		int _nextBallCollisionGroup = 1;
		std::set<int> _fallenBalls;
	};

} // namespace Billiard
