#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/ComposedSurface/SphereProjectionContributorBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/CircleShapeVisual.h"

namespace Billiard {

	class RollingBallBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _circleRef;
		/// @property
		RefWrapper<Engine::SphereProjectionContributorBehaviour> _sphereProjectionRef;
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _bodyRef;

	private:
		float GetRadius() const;
	};

} // namespace Billiard
