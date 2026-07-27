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

	[[nodiscard]] sf::Vector3f OmegaFromVelocity(sf::Vector2f velocity, float invRadius) {
		return {velocity.y * invRadius, -velocity.x * invRadius, 0.f};
	}

	[[nodiscard]] sf::Vector2f VelocityFromOmega(sf::Vector3f omega, float radius) {
		return {-omega.y * radius, omega.x * radius};
	}

	[[nodiscard]] sf::Vector3f WorldOmegaToLocal(sf::Vector3f worldOmega, sf::Angle worldRotation) {
		const sf::Vector2f localXY = RotateInverse2D({worldOmega.x, worldOmega.y}, worldRotation);
		return {localXY.x, localXY.y, 0.f};
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

	const float dtSeconds = dt.asSeconds();
	const sf::Angle worldRotation = WorldRotationFromTransform(node->GetWorldTransform());
	const float invRadius = 1.f / radius;

	sf::Vector2f velocity = body->GetVelocity();
	sf::Vector2f rollVelocity = VelocityFromOmega(_spinOmega, radius);
	sf::Vector2f slip = velocity - rollVelocity;
	const float slipLength = slip.length();

	if (slipLength > 1e-6f && _inertiaFactor > 0.f) {
		constexpr float massFactor = 1.f;
		const float slipCorrectionMagnitude = _friction * (1.f / massFactor + 1.f / _inertiaFactor) * dtSeconds;

		sf::Vector2f deltaVelocity;
		sf::Vector2f deltaRollVelocity;

		if (slipCorrectionMagnitude < slipLength) {
			const sf::Vector2f slipDir = slip.normalized();
			const sf::Vector2f deltaU = -slipDir * slipCorrectionMagnitude;
			deltaVelocity = deltaU * (_inertiaFactor / (1 + _inertiaFactor));
			deltaRollVelocity = -deltaU * (1.f / (1 + _inertiaFactor));
		}
		else {
			const sf::Vector2f commonVelocity =
			    (velocity * massFactor + rollVelocity * _inertiaFactor) / (1 + _inertiaFactor);
			deltaVelocity = commonVelocity - velocity;
			deltaRollVelocity = commonVelocity - rollVelocity;
		}

		body->SetVelocity(velocity + deltaVelocity);
		_spinOmega = OmegaFromVelocity(rollVelocity + deltaRollVelocity, invRadius);
	}

	const sf::Vector3f localOmegaDt = WorldOmegaToLocal(_spinOmega * dtSeconds, worldRotation);
	const float angle =
	    std::sqrt(localOmegaDt.x * localOmegaDt.x + localOmegaDt.y * localOmegaDt.y + localOmegaDt.z * localOmegaDt.z);
	if (angle < 1e-8f) {
		return;
	}

	sphereProjection->MultiplySphereOrientation(localOmegaDt / angle, angle);
}

float Billiard::RollingBallBehaviour::GetRadius() const {
	if (!_circleRef) {
		return 0.f;
	}
	return _circleRef.Get()->GetRadius();
}

void Billiard::RollingBallBehaviour::SetVerticalSpin(sf::Angle direction, float value) {
	const float radius = GetRadius();
	if (radius <= 0.f) {
		return;
	}

	const float angleRadians = direction.asRadians();
	const sf::Vector2f rollVelocity{std::cos(angleRadians) * value, std::sin(angleRadians) * value};
	_spinOmega = OmegaFromVelocity(rollVelocity, 1.f / radius);
}
