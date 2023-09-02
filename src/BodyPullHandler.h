#pragma once
#include <memory>
#include <SFML/Graphics/Drawable.hpp>
#include "AbstractBody.h"

class BodyPullHandler : public sf::Drawable {
public:
	void enableDebugDraw(bool enable) { mIsDebugDrawEnabled = enable; }
	void onMouseButtonPress(const sf::Event::MouseButtonEvent& event);
	void onMouseButtonRelease(const sf::Event::MouseButtonEvent& event);
	void onMouseMove(const sf::Event::MouseMoveEvent& event);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	bool mIsDebugDrawEnabled = true;
	std::weak_ptr<AbstractBody> mPullingBody;
	std::shared_ptr<sf::Vector2f> mDraggingForce;
};