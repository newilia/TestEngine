#pragma once

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

namespace Engine {

	class SphereProjectionContributorBehaviour : public ComposedSurfaceContributorBehaviour
	{
		META_CLASS()

	public:
		[[nodiscard]] bool IsContributorEnabled() const override;
		[[nodiscard]] ComposedSurfaceContributorKind GetContributorKind() const override;
		[[nodiscard]] bool TryContributeSphereProjection(ComposedSurfaceSphereProjectionData& out) const override;

		[[nodiscard]] sf::Angle GetRotation() const;
		void SetRotation(sf::Angle value);

		[[nodiscard]] sf::Vector2f GetSphereUvOffset() const;
		void SetSphereUvOffset(sf::Vector2f value);

	private:
		/// @property
		bool _isEnabled = true;
		/// @property(dragSpeed=0.05f)
		sf::Angle _rotation{};
		/// @property(dragSpeed=0.001f)
		sf::Vector2f _sphereUvOffset{};
	};

} // namespace Engine
