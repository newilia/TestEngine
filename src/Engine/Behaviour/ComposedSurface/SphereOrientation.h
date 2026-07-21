#pragma once

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector3.hpp>

namespace Engine {

	struct SphereOrientationQuat
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
		float w = 1.f;
	};

	[[nodiscard]] SphereOrientationQuat SphereOrientationIdentity();
	[[nodiscard]] SphereOrientationQuat SphereOrientationFromAxisAngle(sf::Vector3f axis, float angleRadians);
	[[nodiscard]] SphereOrientationQuat SphereOrientationMultiply(SphereOrientationQuat lhs, SphereOrientationQuat rhs);
	[[nodiscard]] SphereOrientationQuat SphereOrientationNormalize(SphereOrientationQuat q);

	void SphereOrientationToEulerYxz(
	    SphereOrientationQuat q, float& yawRadians, float& pitchRadians, float& rollRadians);
	[[nodiscard]] SphereOrientationQuat SphereOrientationFromEulerYxz(
	    float yawRadians, float pitchRadians, float rollRadians);

} // namespace Engine
