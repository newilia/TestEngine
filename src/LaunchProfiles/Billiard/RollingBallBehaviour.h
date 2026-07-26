#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/ComposedSurface/SphereProjectionContributorBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <SFML/System/Vector3.hpp>

namespace Billiard {

	class RollingBallBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

	public:
		void SetVerticalSpin(sf::Angle direction, float value);

	private:
		float GetRadius() const;

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _circleRef;
		/// @property
		RefWrapper<Engine::SphereProjectionContributorBehaviour> _sphereProjectionRef;
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _bodyRef;

		/// @property(minValue=0.f)
		float _friction = 10.f;
		/// @property
		sf::Vector3f _spinOmega{};
		/// @property
		float _impulseFactor = 1.f;
	};

} // namespace Billiard
