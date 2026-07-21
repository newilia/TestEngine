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
		return true;
	}

	sf::Vector2f SphereProjectionContributorBehaviour::GetSphereUvOffset() const {
		return _sphereUvOffset;
	}

	void SphereProjectionContributorBehaviour::SetSphereUvOffset(sf::Vector2f value) {
		_sphereUvOffset = value;
	}

} // namespace Engine
