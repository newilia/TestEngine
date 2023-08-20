#include "UserInput.h"
#include "BodyPullHandler.h"
#include "PhysicsHandler.h"

extern float SIM_SPEED;
extern bool SIM_PAUSE;

void UserInput::handleEvent(const sf::Event& event) {
	switch (event.type) {
	case sf::Event::MouseButtonPressed:
		onMouseButtonPress(event.mouseButton);
		break;
	case sf::Event::MouseMoved:
		onMouseMove(event.mouseMove);
		break;
	case sf::Event::MouseButtonReleased:
		onMouseButtonRelease(event.mouseButton);
		break;
	case sf::Event::KeyPressed:
		onKeyPress(event.key);
		break;
	case sf::Event::KeyReleased:
		onKeyRelease(event.key);
		break;
	case sf::Event::Closed:
		std::exit(EXIT_SUCCESS);
	default:
		break;
	}
}

void UserInput::onMouseButtonPress(const sf::Event::MouseButtonEvent& event) {
	BodyPullHandler::getInstance()->onMouseButtonPress(event);
}

void UserInput::onMouseButtonRelease(const sf::Event::MouseButtonEvent& event) {
	BodyPullHandler::getInstance()->onMouseButtonRelease(event);
}

void UserInput::onMouseMove(const sf::Event::MouseMoveEvent& event) {
	BodyPullHandler::getInstance()->onMouseMove(event);
}

void UserInput::onKeyPress(const sf::Event::KeyEvent& key) {
	switch (key.code) {
	case sf::Keyboard::Equal:
		SIM_SPEED *= 2;
		break;
	case sf::Keyboard::Hyphen:
		SIM_SPEED *= 0.5f;
		break;
	case sf::Keyboard::Num0:
		SIM_PAUSE = !SIM_PAUSE;
		break;
	default: break;
	}
}

void UserInput::onKeyRelease(const sf::Event::KeyEvent& key) {
}