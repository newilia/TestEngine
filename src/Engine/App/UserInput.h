#pragma once
#include "Engine/Core/Delegates.h"

#include <SFML/Window/Event.hpp>

#include <functional>
#include <set>

class UserInput
{
public:
	using EventHandler = std::function<void(const sf::Event&)>;
	void HandleEvent(const sf::Event& event);
	void AttachEventHandler(std::unique_ptr<IDelegate<sf::Event>>&& delegatePtr);

private:
	// void onMouseButtonPress(const sf::Event::MouseButtonEvent& event);
	// void onMouseButtonRelease(const sf::Event::MouseButtonEvent& event);
	// void onMouseMove(const sf::Event::MouseMoveEvent& event);
	// void onKeyPress(const sf::Event::KeyEvent& key);
	// void onKeyRelease(const sf::Event::KeyEvent& key);
	std::set<std::unique_ptr<IDelegate<sf::Event>>> _eventHandlers;
};
