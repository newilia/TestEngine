#pragma once

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/Visual.h"

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

private:
	/// @property
	std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>> _contributors;
};
