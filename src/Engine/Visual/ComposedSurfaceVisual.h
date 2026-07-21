#pragma once

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include <vector>

class ComposedSurfaceVisual : public Visual
{
	META_CLASS()
	META_PROPERTY_BASE(Visual)

public:
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	sf::FloatRect GetLocalBounds() const override;
	sf::FloatRect GetGlobalBounds() const override;
	const sf::Transform* GetTransform() const override;

	[[nodiscard]] const std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>>& GetContributors() const;
	void SetContributors(std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>> value);

	[[nodiscard]] sf::Vector2f GetSize() const;
	void SetSize(sf::Vector2f value);

	[[nodiscard]] sf::Vector2f GetOrigin() const;
	void SetOrigin(sf::Vector2f value);

	[[nodiscard]] bool UsesSphereProjection() const;

private:
	void SyncQuad() const;

	/// @property
	std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>> _contributors;
	/// @property(dragSpeed=1.f, minValue=0.01f)
	sf::Vector2f _size{40.f, 40.f};
	/// @property(dragSpeed=1.f)
	sf::Vector2f _origin{20.f, 20.f};

	mutable sf::RectangleShape _quad;
};
