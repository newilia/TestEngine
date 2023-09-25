#include "UserInput.h"
#include "BodyPullHandler.h"
#include "PhysicsHandler.h"


void UserInput::handleEvent(const sf::Event& event) {
	//switch (event.type) {
	//case sf::Event::MouseButtonPressed:
	//	onMouseButtonPress(event.mouseButton);
	//	break;
	//case sf::Event::MouseMoved:
	//	onMouseMove(event.mouseMove);
	//	break;
	//case sf::Event::MouseButtonReleased:
	//	onMouseButtonRelease(event.mouseButton);
	//	break;
	//case sf::Event::KeyPressed:
	//	onKeyPress(event.key);
	//	break;
	//case sf::Event::KeyReleased:
	//	onKeyRelease(event.key);
	//	break;
	//default:
	//	break;
	//}

	for (auto it = mEventHandlers.begin(); it != mEventHandlers.end(); ) {
		if (it->get()->expired()) {
			it = mEventHandlers.erase(it);
			continue;
		}
		it++->get()->operator()(event);
	}
}

void UserInput::attachEventHandler(std::unique_ptr<IDelegate<sf::Event>>&& delegatePtr) {
	mEventHandlers.emplace(std::move(delegatePtr));
}

//void UserInput::onMouseButtonPress(const sf::Event::MouseButtonEvent& event) {
//}
//
//void UserInput::onMouseButtonRelease(const sf::Event::MouseButtonEvent& event) {
//}
//
//void UserInput::onMouseMove(const sf::Event::MouseMoveEvent& event) {
//}
//
//void UserInput::onKeyPress(const sf::Event::KeyEvent& key) {
//}
//
//void UserInput::onKeyRelease(const sf::Event::KeyEvent& key) {
//}