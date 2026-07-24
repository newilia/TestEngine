#pragma once

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

		void PositionOn(const RefWrapper<SceneNode>& anchor);
		void SetRotation(sf::Angle angle);
		void MoveLongitudinal(float delta);
		void MoveLateral(float delta);

	private:
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _bodyRef;

		RefWrapper<SceneNode> _anchorRef;
		bool _isActive = false;
	};

} // namespace Billiard
