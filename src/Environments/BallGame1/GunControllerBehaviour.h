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
		void SetRotationLimits(sf::Angle min, sf::Angle max);

	private:
		/// @property(dragSpeed=0.05f)
		float _rotationSpeed = 0.0f;
		// TODO add codegen support @property(dragSpeed=0.01f)
		std::pair<sf::Angle, sf::Angle> _rotationLimits{};
	};
} // namespace BallGame1
