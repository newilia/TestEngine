#pragma once
#include <functional>
#include <set>

#include "Updateable.h"
#include "Delegates.h"


class UserInput {
public:
	using EventHandler = std::function<void(const sf::Event&)>;
	void handleEvent(const sf::Event& event);
	void attachCustomHandler(std::unique_ptr<IDelegate<sf::Event>>&& delegatePtr);

private:
	//void onMouseButtonPress(const sf::Event::MouseButtonEvent& event);
	//void onMouseButtonRelease(const sf::Event::MouseButtonEvent& event);
	//void onMouseMove(const sf::Event::MouseMoveEvent& event);
	//void onKeyPress(const sf::Event::KeyEvent& key);
	//void onKeyRelease(const sf::Event::KeyEvent& key);

	std::set<std::unique_ptr<IDelegate<sf::Event>>> mEventHandlers;
};