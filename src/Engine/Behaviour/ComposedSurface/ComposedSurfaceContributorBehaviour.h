#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceTypes.h"

namespace Engine {

	class ComposedSurfaceContributorBehaviour : public Behaviour
	{
	public:
		~ComposedSurfaceContributorBehaviour() override = default;

		[[nodiscard]] virtual bool IsContributorEnabled() const = 0;
		[[nodiscard]] virtual ComposedSurfaceContributorKind GetContributorKind() const = 0;

		[[nodiscard]] virtual bool TryContributeTile(ComposedSurfaceTileData& /*out*/) const {
			return false;
		}

		[[nodiscard]] virtual bool TryContributeSphereProjection(ComposedSurfaceSphereProjectionData& /*out*/) const {
			return false;
		}
	};

} // namespace Engine
