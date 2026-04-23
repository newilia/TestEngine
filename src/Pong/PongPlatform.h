#pragma once

#include "PlatformControllerBase.h"

#include "Engine/common.h"
#include "Engine/SceneNode.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics.hpp>

#include <memory>

namespace sf {
class ConvexShape;
}

/// Платформа Pong: геометрия и физика — через ShapeCollider + PhysicalBehaviour на ноде.
class PongPlatform : public std::enable_shared_from_this<PongPlatform>
{
public:
	explicit PongPlatform(std::shared_ptr<SceneNode> node);

	void registerTickBehaviour();

	void Init();

	void setShapeDimensions(sf::Vector2f size, float curveness, float rotationDeg);

	void Update(const sf::Time& dt);

	std::shared_ptr<SceneNode> GetNode() const { return _node; }

	sf::ConvexShape* GetShape() const;

	sf::FloatRect GetBbox() const;

	sf::Vector2f GetPosGlobal() const { return _node->GetPosGlobal(); }

	void setName(const std::string& name) { _node->setName(name); }

	shared_ptr<PlatformControllerBase> getController() const { return _controller; }

	void setController(const shared_ptr<PlatformControllerBase>& controller) { _controller = controller; }

	auto GetPhysicalComponent() const { return _node->GetPhysicalComponent(); }

private:
	std::shared_ptr<SceneNode> _node;
	shared_ptr<PlatformControllerBase> _controller;
};
