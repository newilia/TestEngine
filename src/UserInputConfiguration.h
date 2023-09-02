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
		case sf::Event::MouseButtonPressed: {
			sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
			if (event.mouseButton.button == sf::Mouse::Button::Left) {
				interface->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::SOFT);
			}
			else if (event.mouseButton.button == sf::Mouse::Button::Right) {
				interface->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::HARD);
			}
			break;
		}

		case sf::Event::MouseButtonReleased:
			interface->getBodyPullHandler()->stopPull();
			break;

		case sf::Event::MouseMoved:
			interface->getBodyPullHandler()->setPullDestination(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
			break;
		default:
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
