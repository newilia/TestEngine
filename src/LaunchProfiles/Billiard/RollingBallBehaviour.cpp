#include "RollingBallBehaviour.h"

#include "RollingBallBehaviour.generated.hpp"

#include <numbers>

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

	const float circumference = 2.f * std::numbers::pi_v<float> * radius;
	const sf::Vector2f velocity = body->GetVelocity();
	const float dtSeconds = dt.asSeconds();

	sf::Vector2f uvOffset = sphereProjection->GetSphereUvOffset();
	uvOffset.x -= velocity.x * dtSeconds / circumference;
	uvOffset.y -= velocity.y * dtSeconds / circumference;
	sphereProjection->SetSphereUvOffset(uvOffset);
}

float Billiard::RollingBallBehaviour::GetRadius() const {
	if (!_circleRef) {
		return 0.f;
	}
	return _circleRef.Get()->GetRadius();
}
