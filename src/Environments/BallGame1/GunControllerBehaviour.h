#pragma once
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"

namespace BallGame1 {
	class GunControllerBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnUpdate(const sf::Time& dt) override;
		void OnEvent(const sf::Event& event) override;

		void SetRotationSpeed(float rotationSpeed);

	private:
		/// @property(dragSpeed=0.001f)
		float _rotationSpeed = 0.0f;
	};
} // namespace BallGame1
