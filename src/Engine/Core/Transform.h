#pragma once

#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

/// Local TRS relative to the parent node. Owned by `SceneNode`; does not reference the node.
/// World-matrix invalidation is handled by `SceneNode` when local transform changes.
class Transform final : public Engine::IPropertiesProvider
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
	/// @setter(dragSpeed=0.1f)
	void SetRotation(sf::Angle angle);

	/// @getter
	sf::Vector2f GetOrigin() const;
	/// @setter
	void SetOrigin(sf::Vector2f v);

private:
	sf::Transformable _transformable;
};
