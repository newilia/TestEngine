#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

/// Per-node spatial transform; held by `SceneNode` as local TRS relative to the parent.
/// Mutations use `setLocal*` only so hierarchy caches stay consistent (no public `sf::Transformable`).
class Transform final : public EntityOnNode
{
public:
	void BuildPropertyTree(Engine::PropertyBuilder& b) override;

	[[nodiscard]] sf::Transform getTransform() const;

	[[nodiscard]] sf::Vector2f getLocalPosition() const;
	void setLocalPosition(sf::Vector2f v);

	[[nodiscard]] sf::Vector2f getLocalScale() const;
	void setLocalScale(sf::Vector2f v);

	[[nodiscard]] sf::Angle getLocalRotation() const;
	void setLocalRotation(sf::Angle angle);

	[[nodiscard]] sf::Vector2f getLocalOrigin() const;
	void setLocalOrigin(sf::Vector2f v);

private:
	void notifyTransformChanged();

	sf::Transformable _transformable;
};
