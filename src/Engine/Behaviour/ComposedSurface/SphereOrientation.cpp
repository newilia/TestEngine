#include "Engine/Behaviour/ComposedSurface/SphereOrientation.h"

#include <cmath>

namespace Engine {

	namespace {

		[[nodiscard]] float LengthSquared(sf::Vector3f v) {
			return v.x * v.x + v.y * v.y + v.z * v.z;
		}

	} // namespace

	SphereOrientationQuat SphereOrientationIdentity() {
		return {};
	}

	SphereOrientationQuat SphereOrientationFromAxisAngle(sf::Vector3f axis, float angleRadians) {
		const float axisLengthSquared = LengthSquared(axis);
		if (axisLengthSquared < 1e-12f) {
			return SphereOrientationIdentity();
		}
		const float invLength = 1.f / std::sqrt(axisLengthSquared);
		axis *= invLength;
		const float halfAngle = angleRadians * 0.5f;
		const float s = std::sin(halfAngle);
		return {axis.x * s, axis.y * s, axis.z * s, std::cos(halfAngle)};
	}

	SphereOrientationQuat SphereOrientationMultiply(SphereOrientationQuat lhs, SphereOrientationQuat rhs) {
		return {
		    lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		    lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
		    lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
		    lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
		};
	}

	SphereOrientationQuat SphereOrientationNormalize(SphereOrientationQuat q) {
		const float lengthSquared = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
		if (lengthSquared < 1e-12f) {
			return SphereOrientationIdentity();
		}
		const float invLength = 1.f / std::sqrt(lengthSquared);
		return {q.x * invLength, q.y * invLength, q.z * invLength, q.w * invLength};
	}

	void SphereOrientationToEulerYxz(
	    SphereOrientationQuat q, float& yawRadians, float& pitchRadians, float& rollRadians) {
		const float sinPitch = 2.f * (q.w * q.y - q.z * q.x);
		pitchRadians = std::asin(std::clamp(sinPitch, -1.f, 1.f));

		const float cosPitch = std::cos(pitchRadians);
		if (std::abs(cosPitch) > 1e-6f) {
			yawRadians = std::atan2(2.f * (q.w * q.x + q.y * q.z), 1.f - 2.f * (q.x * q.x + q.y * q.y));
			rollRadians = std::atan2(2.f * (q.w * q.z + q.x * q.y), 1.f - 2.f * (q.y * q.y + q.z * q.z));
		}
		else {
			yawRadians = std::atan2(2.f * (q.x * q.z + q.w * q.y), 1.f - 2.f * (q.y * q.y + q.z * q.z));
			rollRadians = 0.f;
		}
	}

	SphereOrientationQuat SphereOrientationFromEulerYxz(float yawRadians, float pitchRadians, float rollRadians) {
		const float cy = std::cos(yawRadians * 0.5f);
		const float sy = std::sin(yawRadians * 0.5f);
		const float cp = std::cos(pitchRadians * 0.5f);
		const float sp = std::sin(pitchRadians * 0.5f);
		const float cr = std::cos(rollRadians * 0.5f);
		const float sr = std::sin(rollRadians * 0.5f);

		return {
		    cy * sp * cr + sy * cp * sr,
		    sy * cp * cr - cy * sp * sr,
		    cy * cp * sr - sy * sp * cr,
		    cy * cp * cr + sy * sp * sr,
		};
	}

} // namespace Engine
