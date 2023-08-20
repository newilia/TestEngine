#pragma once
#include "Updateable.h"

class UserInput {
public:
	void handleEvent(const sf::Event& event);
private:
	void onMouseButtonPress(const sf::Event::MouseButtonEvent& event);
	void onMouseButtonRelease(const sf::Event::MouseButtonEvent& event);
	void onMouseMove(const sf::Event::MouseMoveEvent& event);
	void onKeyPress(const sf::Event::KeyEvent& key);
	void onKeyRelease(const sf::Event::KeyEvent& key);
};