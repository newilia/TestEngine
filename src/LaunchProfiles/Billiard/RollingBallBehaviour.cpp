#include "RollingBallBehaviour.h"

#include "Engine/Core/SceneNode.h"
#include "RollingBallBehaviour.generated.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <cmath>

namespace {

	[[nodiscard]] sf::Angle WorldRotationFromTransform(const sf::Transform& worldTransform) {
		const sf::Vector2f origin = worldTransform.transformPoint(sf::Vector2f{});
		const sf::Vector2f xAxis = worldTransform.transformPoint(sf::Vector2f{1.f, 0.f}) - origin;
		return sf::radians(std::atan2(xAxis.y, xAxis.x));
	}

	[[nodiscard]] sf::Vector2f RotateInverse2D(sf::Vector2f v, sf::Angle worldRotation) {
		const float angle = -worldRotation.asRadians();
		const float c = std::cos(angle);
		const float s = std::sin(angle);
		return {v.x * c - v.y * s, v.x * s + v.y * c};
	}

} // namespace

void Billiard::RollingBallBehaviour::OnUpdate(const sf::Time& dt) {
	const auto sphereProjection = _sphereProjectionRef.Get();
	const auto body = _bodyRef.Get();
	const auto node = GetNode();
	if (!sphereProjection || !body || !node) {
		return;
	}

	const float radius = GetRadius();
	if (radius <= 0.f) {
		return;
	}

	const sf::Vector2f velocity = body->GetVelocity();
	const float dtSeconds = dt.asSeconds();
	const sf::Angle worldRotation = WorldRotationFromTransform(node->GetWorldTransform());
	const sf::Vector2f localVelocity = RotateInverse2D(velocity, worldRotation);

	const float invRadius = 1.f / radius;
	const sf::Vector3f rollingOmega(localVelocity.y * invRadius, -localVelocity.x * invRadius, 0.f);
	const sf::Vector3f omegaDt = rollingOmega * dtSeconds;
	const float angle = std::sqrt(omegaDt.x * omegaDt.x + omegaDt.y * omegaDt.y + omegaDt.z * omegaDt.z);
	if (angle < 1e-8f) {
		return;
	}

	sphereProjection->MultiplySphereOrientation(omegaDt / angle, angle);
}

float Billiard::RollingBallBehaviour::GetRadius() const {
	if (!_circleRef) {
		return 0.f;
	}
	return _circleRef.Get()->GetRadius();
}
