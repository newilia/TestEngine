#pragma once
#include <memory>
#include <SFML/Graphics/Drawable.hpp>
#include "Engine/Physics/AbstractBody.h"
#include "UserPullComponent.h"

class BodyPullHandler : public sf::Drawable {
public:
	void enableDebugDraw(bool enable) { mIsDebugDrawEnabled = enable; }
	void startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode);
	void stopPull();
	void setPullDestination(sf::Vector2f dest) const;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	bool mIsDebugDrawEnabled = true;
	std::weak_ptr<AbstractBody> mPullingBody;
	std::shared_ptr<sf::Vector2f> mDraggingForce;
};
