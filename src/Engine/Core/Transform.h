#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

/// Wrapper for sf::Transformable. Held by `SceneNode` as local TRS relative to the parent.
/// Mutations use setters only so hierarchy caches stay consistent (no public `sf::Transformable`).
class Transform final : public EntityOnNode
{
	META_CLASS()

public:
	sf::Transform GetTransform() const;

	/// @getter
	sf::Vector2f GetPosition() const;
	/// @setter
	void SetPosition(sf::Vector2f v);

	/// @getter
	sf::Vector2f GetScale() const;
	/// @setter(dragSpeed=0.01f)
	void SetScale(sf::Vector2f v);

	/// @getter
	sf::Angle GetRotation() const;
	/// @setter
	void SetRotation(sf::Angle angle);

	/// @getter
	sf::Vector2f GetOrigin() const;
	/// @setter
	void SetOrigin(sf::Vector2f v);

private:
	void NotifyTransformChanged();

private:
	sf::Transformable _transformable;
};
