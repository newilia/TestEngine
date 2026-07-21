#include "RollingBallBehaviour.h"

#include "RollingBallBehaviour.generated.hpp"

#include <SFML/System/Angle.hpp>

void Billiard::RollingBallBehaviour::OnUpdate(const sf::Time& dt) {
	const auto sphereProjection = _sphereProjectionRef.Get();
	const auto body = _bodyRef.Get();
	if (!sphereProjection || !body) {
		return;
	}

	const float radius = GetRadius();
	if (radius <= 0.f) {
		return;
	}

	const sf::Vector2f velocity = body->GetVelocity();
	const float dtSeconds = dt.asSeconds();

	sf::Angle yaw = sphereProjection->GetSphereRotationYaw();
	sf::Angle pitch = sphereProjection->GetSphereRotationPitch();
	const float rollRadians = dtSeconds / radius;
	yaw -= sf::radians(velocity.x * rollRadians);
	pitch -= sf::radians(velocity.y * rollRadians);
	sphereProjection->SetSphereRotationYaw(yaw);
	sphereProjection->SetSphereRotationPitch(pitch);
}

float Billiard::RollingBallBehaviour::GetRadius() const {
	if (!_circleRef) {
		return 0.f;
	}
	return _circleRef.Get()->GetRadius();
}
