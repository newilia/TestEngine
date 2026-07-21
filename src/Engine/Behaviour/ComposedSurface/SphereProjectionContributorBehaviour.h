#pragma once

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceTypes.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

namespace Engine {

	META_ENUM(SphereProjectionUnwrap, Horizontal, Vertical);

	class SphereProjectionContributorBehaviour : public ComposedSurfaceContributorBehaviour
	{
		META_CLASS()

	public:
		[[nodiscard]] bool IsContributorEnabled() const override;
		[[nodiscard]] ComposedSurfaceContributorKind GetContributorKind() const override;
		[[nodiscard]] bool TryContributeSphereProjection(ComposedSurfaceSphereProjectionData& out) const override;

		[[nodiscard]] sf::Vector2f GetSphereUvOffset() const;
		void SetSphereUvOffset(sf::Vector2f value);

		[[nodiscard]] SphereProjectionUnwrap GetUnwrap() const;
		void SetUnwrap(SphereProjectionUnwrap value);

		/// @getter
		[[nodiscard]] sf::Angle GetSphereRotationYaw() const;
		/// @setter(dragSpeed=0.1f)
		void SetSphereRotationYaw(sf::Angle value);

		/// @getter
		[[nodiscard]] sf::Angle GetSphereRotationPitch() const;
		/// @setter(dragSpeed=0.1f)
		void SetSphereRotationPitch(sf::Angle value);

	private:
		/// @property
		bool _isEnabled = true;
		/// @property(dragSpeed=0.001f)
		sf::Vector2f _sphereUvOffset{};
		/// @property
		SphereProjectionUnwrap _unwrap = SphereProjectionUnwrap::Horizontal;
		sf::Angle _sphereRotationYaw{};
		sf::Angle _sphereRotationPitch{};
	};

} // namespace Engine
