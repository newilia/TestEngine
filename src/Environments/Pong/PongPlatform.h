#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/common.h"
#include "PlatformControllerBase.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <memory>

namespace sf {
	class ConvexShape;
}

/// Платформа Pong: геометрия и физика — через ShapeCollider + RigidBodyBehaviour на ноде.
class PongPlatform : public std::enable_shared_from_this<PongPlatform>
{
public:
	explicit PongPlatform(std::shared_ptr<SceneNode> node);
	void RegisterTickBehaviour();
	void Init();
	void SetShapeDimensions(sf::Vector2f size, float curveness, float rotationDeg);
	void Run(const sf::Time& dt);

	std::shared_ptr<SceneNode> GetNode() const;

	sf::ConvexShape* GetShape() const;
	sf::FloatRect GetBbox() const;

	sf::Vector2f GetPosGlobal() const;

	void SetName(const std::string& name);

	shared_ptr<PlatformControllerBase> GetController() const;

	void SetController(const shared_ptr<PlatformControllerBase>& controller);

private:
	std::shared_ptr<SceneNode> _node;
	shared_ptr<PlatformControllerBase> _controller;
};
