#pragma once
#include "Engine/Physics/AbstractBody.h"
#include "UserPullComponent.h"

#include <SFML/Graphics/Drawable.hpp>

#include <memory>

class BodyPullHandler : public sf::Drawable
{
public:
	void enableDebugDraw(bool enable) { _isDebugDrawEnabled = enable; }

	void startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode);
	void stopPull();
	void setPullDestination(sf::Vector2f dest) const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	bool _isDebugDrawEnabled = true;
	std::weak_ptr<AbstractBody> _pullingBody;
	std::shared_ptr<sf::Vector2f> _draggingForce;
};
