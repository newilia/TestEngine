#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "LaunchProfiles/Billiard/BilliardBallBehaviour.h"

#include <vector>

namespace Billiard {

	class BilliardPocketBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void RegisterBallFall(BilliardBallBehaviour& ball);

		Signal<int>& GetOnBallFallSignal() const;

	private:
		/// @property
		RefWrapper<ShapeVisualBase> _pocketShape;
		/// @property
		std::vector<RefWrapper<BilliardBallBehaviour>> _ballsInPocket;

		mutable Signal<int> _onBallFall;
	};

} // namespace Billiard
