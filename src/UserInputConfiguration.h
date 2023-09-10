#pragma once
#include "EngineInterface.h"
#include "UserInput.h"

inline void initUserInputHandlers() {
	auto ei = EI();
	auto userInput = ei->getUserInput();
	auto scene = ei->getScene();
	auto builder = ei->getSceneBuilder();

	userInput->attachCustomHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::R) {
			auto scene = ei->getSceneBuilder()->buildScene();
			ei->setScene(scene);
			ei->setSimPaused(false);
			ei->setSimSpeedMultiplier(1.f);
		}
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		switch (event.type) {
		case sf::Event::MouseButtonPressed: {
			sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
			if (event.mouseButton.button == sf::Mouse::Button::Left) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::FORCE);
			}
			else if (event.mouseButton.button == sf::Mouse::Button::Right) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::POSITION);
			}
			else if (event.mouseButton.button == sf::Mouse::Button::Middle) {
				ei->getBodyPullHandler()->startPull(mousePos, UserPullComponent::PullMode::VELOCITY);
			}
			break;
		}

		case sf::Event::MouseButtonReleased:
			ei->getBodyPullHandler()->stopPull();
			break;

		case sf::Event::MouseMoved:
			ei->getBodyPullHandler()->setPullDestination(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
			break;
		default:
			break;
		}
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code) {
			case sf::Keyboard::Equal:
				ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() * 2);
				break;
			case sf::Keyboard::Hyphen:
				ei->setSimSpeedMultiplier(ei->getSimSpeedMultiplier() * 0.5f);
				break;
			case sf::Keyboard::Num0:
				ei->setSimPaused(!ei->isSimPaused());
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

	userInput->attachCustomHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G) {
			ei->getPhysicsHandler()->setGravityEnabled(!ei->getPhysicsHandler()->isGravityEnabled());
		}
	}));

	userInput->attachCustomHandler(createDelegate<sf::Event>([ei](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) {
			ei->setDebugDrawEnabled(!ei->isDebugDrawEnabled());
		}
	}));
}
