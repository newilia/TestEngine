#include "Engine/Visual/ComposedSurfaceVisual.h"

#include "ComposedSurfaceVisual.generated.hpp"
#include "Engine/Visual/ComposedSurfaceDraw.h"

void ComposedSurfaceVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	Engine::DrawComposedSurface(*this, target, states);
}

bool ComposedSurfaceVisual::HitTest(const sf::Vector2f& worldPoint) const {
	const sf::FloatRect bounds = GetGlobalBounds();
	return bounds.contains(worldPoint);
}

sf::FloatRect ComposedSurfaceVisual::GetLocalBounds() const {
	return {{0.f, 0.f}, {40.f, 40.f}};
}

sf::FloatRect ComposedSurfaceVisual::GetGlobalBounds() const {
	if (const auto node = GetNode()) {
		return node->GetWorldTransform().transformRect(GetLocalBounds());
	}
	return GetLocalBounds();
}

const sf::Transform* ComposedSurfaceVisual::GetTransform() const {
	if (const auto node = GetNode()) {
		return &node->GetWorldTransform();
	}
	return nullptr;
}

const std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>>&
ComposedSurfaceVisual::GetContributors() const {
	return _contributors;
}

void ComposedSurfaceVisual::SetContributors(
    std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>> value) {
	_contributors = std::move(value);
}
