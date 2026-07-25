#pragma once

#include "BilliardBallBehaviour.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/System/Angle.hpp>

namespace Billiard {

	class BilliardCueBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void Activate();
		void Deactivate();

		void PositionOnNode(const std::shared_ptr<SceneNode>& node);
		void SetRotation(sf::Angle angle);
		void SetLongitudinalPosition(float position);
		void MoveLongitudinal(float delta);
		void SetLateralPosition(float position);
		void MoveLateral(float delta);
		void SetVerticalSpin(float spin);

	private:
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _bodyRef;

	private:
		bool _isActive = false;
		float _verticalSpin = 0.f;
	};

} // namespace Billiard
