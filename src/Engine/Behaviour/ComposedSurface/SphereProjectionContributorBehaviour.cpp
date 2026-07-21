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
		out.sphereRotationYawRadians = _sphereRotationYaw.asRadians();
		out.sphereRotationPitchRadians = _sphereRotationPitch.asRadians();
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

	sf::Angle SphereProjectionContributorBehaviour::GetSphereRotationYaw() const {
		return _sphereRotationYaw;
	}

	void SphereProjectionContributorBehaviour::SetSphereRotationYaw(sf::Angle value) {
		_sphereRotationYaw = value;
	}

	sf::Angle SphereProjectionContributorBehaviour::GetSphereRotationPitch() const {
		return _sphereRotationPitch;
	}

	void SphereProjectionContributorBehaviour::SetSphereRotationPitch(sf::Angle value) {
		_sphereRotationPitch = value;
	}

} // namespace Engine
