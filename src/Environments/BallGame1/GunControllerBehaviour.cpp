#include "GunControllerBehaviour.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "GunControllerBehaviour.generated.hpp"
#include "SFML/Window/Event.hpp"

namespace BallGame1 {
	void GunControllerBehaviour::OnInit() {
		EventHandlerBehaviourBase::OnInit();
	}

	void GunControllerBehaviour::OnUpdate(const sf::Time& dt) {
		EventHandlerBehaviourBase::OnUpdate(dt);
	}

	void GunControllerBehaviour::OnEvent(const sf::Event& event) {
		if (auto e = event.getIf<sf::Event::MouseMovedRaw>()) {
			if (auto node = GetNode()) {
				auto angle = node->GetLocalTransform()->GetRotation() + sf::radians(e->delta.x * _rotationSpeed);
				node->GetLocalTransform()->SetRotation(angle);
			}
		}
	}

	void GunControllerBehaviour::SetRotationSpeed(float rotationSpeed) {
		_rotationSpeed = rotationSpeed;
	}
} // namespace BallGame1
