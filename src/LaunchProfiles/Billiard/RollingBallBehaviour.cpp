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
	const sf::Vector2f slip = velocity - VelocityFromOmega(_spinOmega, radius);

	if (_friction > 0.f && dtSeconds > 0.f) {
		sf::Vector2f correction = slip * (_friction * dtSeconds);
		const float slipLenSq = slip.x * slip.x + slip.y * slip.y;
		const float correctionLenSq = correction.x * correction.x + correction.y * correction.y;
		if (correctionLenSq > slipLenSq && slipLenSq > 0.f) {
			const float scale = std::sqrt(slipLenSq / correctionLenSq);
			correction *= scale;
		}

		const float inertiaFactor = std::max(_impulseFactor, 0.f);
		const float invInertiaPlusOne = 1.f / (inertiaFactor + 1.f);
		const sf::Vector2f linearCorrection = correction * (inertiaFactor * invInertiaPlusOne);
		const sf::Vector2f spinVelocityCorrection = correction * invInertiaPlusOne;

		velocity -= linearCorrection;
		_spinOmega += OmegaFromVelocity(spinVelocityCorrection, invRadius);
		_spinOmega.z = 0.f;

		body->SetVelocity(velocity);
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
