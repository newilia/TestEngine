#pragma once
#include "GlobalInterface.h"
#include "UserInput.h"

inline void initUserInputHandlers() {
	auto interface = GlobalInterface::getInstance();
	auto userInput = interface->getUserInput();
	auto scene = interface->getScene();
	auto builder = interface->getSceneBuilder();

	userInput->attachCustomHandler(createDelegate<sf::Event>([interface](sf::Event event) {
		if (!(event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::R)) {
			return;
		}
		auto scene = interface->getSceneBuilder()->buildScene();
		interface->setScene(scene);
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([interface](sf::Event event) {
		switch (event.type) {
		case sf::Event::MouseButtonPressed:
			interface->getBodyPullHandler()->onMouseButtonPress(event.mouseButton);
			break;

		case sf::Event::MouseButtonReleased:
			interface->getBodyPullHandler()->onMouseButtonRelease(event.mouseButton);
			break;

		case sf::Event::MouseMoved:
			interface->getBodyPullHandler()->onMouseMove(event.mouseMove);
			break;
		}
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([interface](sf::Event event) {
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code) {
			case sf::Keyboard::Equal:
				interface->setSimSpeedMultiplier(interface->getSimSpeedMultiplier() * 2);
				break;
			case sf::Keyboard::Hyphen:
				interface->setSimSpeedMultiplier(interface->getSimSpeedMultiplier() * 0.5f);
				break;
			case sf::Keyboard::Num0:
				interface->setSimPaused(!interface->isSimPaused());
				break;
			default: break;
			}
		}
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([](sf::Event event) {
		if (event.type == sf::Event::Closed) {
			std::exit(EXIT_SUCCESS);
		}
	}));
	
}
