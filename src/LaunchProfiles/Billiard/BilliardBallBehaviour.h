#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "RollingBallBehaviour.h"

namespace Billiard {

	class BilliardBallBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;
		void OnEvent(const sf::Event& event) override;

	public:
		void SetBallNumber(int ballNumber);
		[[nodiscard]] int GetBallNumber() const;
		[[nodiscard]] bool IsCue() const;
		[[nodiscard]] bool IsEight() const;
		[[nodiscard]] bool IsStriped() const;
		[[nodiscard]] float GetRadius() const;
		std::shared_ptr<PhysicsBodyBehaviour> GetPhysicsBody() const;
		std::shared_ptr<RollingBallBehaviour> GetRollingBallBehaviour() const;
		void SetBallInHand(const sf::FloatRect& allowedFreeMoveArea);
		void ResetBallInHand();
		Signal<>& GetOnGrabSignal() const;
		Signal<>& GetOnReleaseSignal() const;

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
		RefWrapper<RollingBallBehaviour> _rollingBall;
		/// @property
		float _fallAnimationDuration = 0.3f;

	private:
		void OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button);
		void OnMouseButtonReleased(const sf::Vector2i& position);
		void OnMouseMoved(const sf::Vector2i& position);

		bool _isFalling = false;
		float _fallAnimationProgress = 0.f;
		float _initialLightingStrength = 1.f;
		std::optional<sf::Vector2f> _dragStartPosition;
		std::optional<sf::FloatRect> _ballInHandArea;
		PhysicsBodyBehaviour::GroupSet _overlapGroupsBeforeGrab;
		PhysicsBodyBehaviour::GroupSet _collisionGroupsBeforeGrab;
		mutable Signal<> _onGrabSignal;
		mutable Signal<> _onReleaseSignal;
	};

} // namespace Billiard
