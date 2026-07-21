#include "Engine/Behaviour/ComposedSurface/SphereProjectionContributorBehaviour.h"

#include "SphereProjectionContributorBehaviour.generated.hpp"

namespace Engine {

	bool SphereProjectionContributorBehaviour::IsContributorEnabled() const {
		return _isEnabled;
	}

	ComposedSurfaceContributorKind SphereProjectionContributorBehaviour::GetContributorKind() const {
		return ComposedSurfaceContributorKind::SphereProjection;
	}

	bool SphereProjectionContributorBehaviour::TryContributeSphereProjection(
	    ComposedSurfaceSphereProjectionData& out) const {
		if (!_isEnabled) {
			return false;
		}
		out.active = true;
		out.sphereUvOffset = _sphereUvOffset;
		out.sphereUnwrap = static_cast<int>(_unwrap);
		out.sphereOrientationX = _sphereOrientation.x;
		out.sphereOrientationY = _sphereOrientation.y;
		out.sphereOrientationZ = _sphereOrientation.z;
		out.sphereOrientationW = _sphereOrientation.w;
		return true;
	}

	sf::Vector2f SphereProjectionContributorBehaviour::GetSphereUvOffset() const {
		return _sphereUvOffset;
	}

	void SphereProjectionContributorBehaviour::SetSphereUvOffset(sf::Vector2f value) {
		_sphereUvOffset = value;
	}

	SphereProjectionUnwrap SphereProjectionContributorBehaviour::GetUnwrap() const {
		return _unwrap;
	}

	void SphereProjectionContributorBehaviour::SetUnwrap(SphereProjectionUnwrap value) {
		_unwrap = value;
	}

	SphereOrientationQuat SphereProjectionContributorBehaviour::GetSphereOrientation() const {
		return _sphereOrientation;
	}

	void SphereProjectionContributorBehaviour::SetSphereOrientation(SphereOrientationQuat value) {
		_sphereOrientation = SphereOrientationNormalize(value);
	}

	void SphereProjectionContributorBehaviour::MultiplySphereOrientation(sf::Vector3f axis, float angleRadians) {
		_sphereOrientation = SphereOrientationNormalize(
		    SphereOrientationMultiply(_sphereOrientation, SphereOrientationFromAxisAngle(axis, angleRadians)));
	}

	sf::Angle SphereProjectionContributorBehaviour::GetSphereRotationYaw() const {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		return sf::radians(yawRadians);
	}

	void SphereProjectionContributorBehaviour::SetSphereRotationYaw(sf::Angle value) {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		_sphereOrientation = SphereOrientationFromEulerYxz(value.asRadians(), pitchRadians, rollRadians);
	}

	sf::Angle SphereProjectionContributorBehaviour::GetSphereRotationPitch() const {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		return sf::radians(pitchRadians);
	}

	void SphereProjectionContributorBehaviour::SetSphereRotationPitch(sf::Angle value) {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		_sphereOrientation = SphereOrientationFromEulerYxz(yawRadians, value.asRadians(), rollRadians);
	}

	sf::Angle SphereProjectionContributorBehaviour::GetSphereRotationRoll() const {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		return sf::radians(rollRadians);
	}

	void SphereProjectionContributorBehaviour::SetSphereRotationRoll(sf::Angle value) {
		float yawRadians = 0.f;
		float pitchRadians = 0.f;
		float rollRadians = 0.f;
		SphereOrientationToEulerYxz(_sphereOrientation, yawRadians, pitchRadians, rollRadians);
		_sphereOrientation = SphereOrientationFromEulerYxz(yawRadians, pitchRadians, value.asRadians());
	}

} // namespace Engine
