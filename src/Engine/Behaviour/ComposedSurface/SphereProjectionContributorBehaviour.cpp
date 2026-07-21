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
		out.rotation = _rotation;
		out.sphereUvOffset = _sphereUvOffset;
		return true;
	}

	sf::Angle SphereProjectionContributorBehaviour::GetRotation() const {
		return _rotation;
	}

	void SphereProjectionContributorBehaviour::SetRotation(sf::Angle value) {
		_rotation = value;
	}

	sf::Vector2f SphereProjectionContributorBehaviour::GetSphereUvOffset() const {
		return _sphereUvOffset;
	}

	void SphereProjectionContributorBehaviour::SetSphereUvOffset(sf::Vector2f value) {
		_sphereUvOffset = value;
	}

} // namespace Engine
