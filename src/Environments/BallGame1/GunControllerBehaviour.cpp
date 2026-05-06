#include "GunControllerBehaviour.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "GunControllerBehaviour.generated.hpp"
#include "SFML/Window/Event.hpp"

#include <algorithm>
#include <cmath>

namespace {

	[[nodiscard]] float DegreesNormalized360(float deg) {
		deg = std::fmod(deg, 360.f);
		if (deg < 0.f) {
			deg += 360.f;
		}
		return deg;
	}

	/// Same orientation as SFML, but comparable to signed limits (e.g. -45° vs 315°).
	[[nodiscard]] float SignedDegrees(sf::Angle a) {
		float deg = a.asDegrees();
		deg = std::fmod(deg + 180.f, 360.f);
		if (deg < 0.f) {
			deg += 360.f;
		}
		return deg - 180.f;
	}

	[[nodiscard]] float CircularDegDist(float a, float b) {
		const float d = std::abs(a - b);
		return std::min(d, 360.f - d);
	}

	/// When lo <= hi in [0,360): single arc. When lo > hi: arc through 0° — [lo,360) U [0,hi].
	[[nodiscard]] float ClampDegreesOnCircle(float angleDeg, float minDeg, float maxDeg) {
		const float lo = DegreesNormalized360(minDeg);
		const float hi = DegreesNormalized360(maxDeg);
		float a = DegreesNormalized360(angleDeg);

		if (lo <= hi) {
			if (a >= lo && a <= hi) {
				return a;
			}
			return CircularDegDist(a, lo) <= CircularDegDist(a, hi) ? lo : hi;
		}

		if (a >= lo || a <= hi) {
			return a;
		}
		return CircularDegDist(a, lo) <= CircularDegDist(a, hi) ? lo : hi;
	}

} // namespace

namespace BallGame1 {
	void GunControllerBehaviour::OnInit() {
		EventHandlerBehaviourBase::OnInit();
	}

	void GunControllerBehaviour::OnUpdate(const sf::Time& dt) {
		EventHandlerBehaviourBase::OnUpdate(dt);
	}

	void GunControllerBehaviour::OnEvent(const sf::Event& event) {
		if (const auto* e = event.getIf<sf::Event::MouseMovedRaw>()) {
			if (auto node = GetNode()) {
				auto xf = node->GetLocalTransform();
				const float deltaRad = e->delta.x * _rotationSpeed;
				const float minDeg = _rotationLimits.first.asDegrees();
				const float maxDeg = _rotationLimits.second.asDegrees();

				if (minDeg <= maxDeg) {
					const float cur = SignedDegrees(xf->GetRotation());
					const float nextDeg = cur + sf::radians(deltaRad).asDegrees();
					const float clamped = std::clamp(nextDeg, minDeg, maxDeg);
					xf->SetRotation(sf::degrees(clamped));
				}
				else {
					const float cur = DegreesNormalized360(xf->GetRotation().asDegrees());
					const float nextDeg = DegreesNormalized360(cur + sf::radians(deltaRad).asDegrees());
					xf->SetRotation(sf::degrees(ClampDegreesOnCircle(nextDeg, minDeg, maxDeg)));
				}
			}
		}
	}

	void GunControllerBehaviour::SetRotationSpeed(float rotationSpeed) {
		_rotationSpeed = rotationSpeed;
	}

	void GunControllerBehaviour::SetRotationLimits(sf::Angle min, sf::Angle max) {
		_rotationLimits = {min, max};
	}
} // namespace BallGame1
