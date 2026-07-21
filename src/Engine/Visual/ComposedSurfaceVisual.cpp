#include "Engine/Visual/ComposedSurfaceVisual.h"

#include "ComposedSurfaceVisual.generated.hpp"
#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceTypes.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/ComposedSurfaceDraw.h"

#include <algorithm>

namespace {

	bool IsPointInsideUnitDisk(sf::Vector2f p) {
		return p.x * p.x + p.y * p.y <= 1.f;
	}

} // namespace

void ComposedSurfaceVisual::SyncQuad() const {
	_quad.setSize(_size);
	_quad.setOrigin(_origin);
	_quad.setPosition({0.f, 0.f});
}

void ComposedSurfaceVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	Engine::DrawComposedSurface(*this, target, states);
}

bool ComposedSurfaceVisual::HitTest(const sf::Vector2f& worldPoint) const {
	const auto node = GetNode();
	if (!node) {
		return false;
	}

	SyncQuad();
	sf::Transform full = node->GetWorldTransform() * _quad.getTransform();
	const sf::Vector2f local = full.getInverse().transformPoint(worldPoint);
	const sf::FloatRect localBounds({{0.f, 0.f}, _size});
	if (!localBounds.contains(local)) {
		return false;
	}

	if (!UsesSphereProjection()) {
		return true;
	}

	sf::Vector2f norm = {local.x / std::max(_size.x, 1e-5f), local.y / std::max(_size.y, 1e-5f)};
	sf::Vector2f p = norm * 2.f - sf::Vector2f{1.f, 1.f};
	const float aspect = _size.x / std::max(_size.y, 1e-5f);
	p.x *= aspect;
	return IsPointInsideUnitDisk(p);
}

sf::FloatRect ComposedSurfaceVisual::GetLocalBounds() const {
	SyncQuad();
	return _quad.getLocalBounds();
}

sf::FloatRect ComposedSurfaceVisual::GetGlobalBounds() const {
	if (const auto node = GetNode()) {
		SyncQuad();
		return (node->GetWorldTransform() * _quad.getTransform()).transformRect(GetLocalBounds());
	}
	return GetLocalBounds();
}

const sf::Transform* ComposedSurfaceVisual::GetTransform() const {
	SyncQuad();
	return &_quad.getTransform();
}

const std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>>&
ComposedSurfaceVisual::GetContributors() const {
	return _contributors;
}

void ComposedSurfaceVisual::SetContributors(
    std::vector<RefWrapper<Engine::ComposedSurfaceContributorBehaviour>> value) {
	_contributors = std::move(value);
}

sf::Vector2f ComposedSurfaceVisual::GetSize() const {
	return _size;
}

void ComposedSurfaceVisual::SetSize(sf::Vector2f value) {
	_size = {std::max(value.x, 0.01f), std::max(value.y, 0.01f)};
}

sf::Vector2f ComposedSurfaceVisual::GetOrigin() const {
	return _origin;
}

void ComposedSurfaceVisual::SetOrigin(sf::Vector2f value) {
	_origin = value;
}

bool ComposedSurfaceVisual::UsesSphereProjection() const {
	for (const RefWrapper<Engine::ComposedSurfaceContributorBehaviour>& ref : _contributors) {
		const auto contributor = ref.Get();
		if (!contributor || !contributor->IsContributorEnabled()) {
			continue;
		}
		if (contributor->GetContributorKind() == Engine::ComposedSurfaceContributorKind::SphereProjection) {
			return true;
		}
	}
	return false;
}
