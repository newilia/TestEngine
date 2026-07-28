#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/CircleShapeVisual.h"

namespace Billiard {

	class BilliardBallBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

	public:
		void SetBallNumber(int ballNumber);
		[[nodiscard]] int GetBallNumber() const;
		[[nodiscard]] bool IsCue() const;
		[[nodiscard]] bool IsEight() const;
		[[nodiscard]] bool IsStriped() const;
		[[nodiscard]] float GetRadius() const;
		std::shared_ptr<PhysicsBodyBehaviour> GetPhysicsBody() const;

		void Appear();
		void PlayFallAnimation();

	private:
		/// @property(minValue=0, maxValue=15)
		int _ballNumber = 0;
		/// @property
		RefWrapper<Engine::TiledTextureContributorBehaviour> _textureContributor;
		/// @property
		RefWrapper<ShapeLightReceiverBehaviour> _lightReceiver;
		/// @property
		RefWrapper<CircleShapeVisual> _ballShape;
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _physicsBody;

		/// @property
		float _fallAnimationDuration = 0.3f;

	private:
		bool _isFalling = false;
		float _fallAnimationProgress = 0.f;
		float _initialLightingStrength = 1.f;
	};

} // namespace Billiard
