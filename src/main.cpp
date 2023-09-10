#include <iostream>
#include <SFML/Graphics.hpp>
#include "BodyPullHandler.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "UserInput.h"
#include "UserInputConfiguration.h"
#include <fmt/format.h>

namespace {
    auto mode = sf::VideoMode(800u, 600u);
    auto windowStyle = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
}

int main() {
    sf::RenderWindow window(mode, "Test Engine", windowStyle);

    auto ei = EI();
    ei->setMainWindow(window);
    ei->setScene(ei->getSceneBuilder()->buildScene());
    ei->getPhysicsHandler()->setSubstepCount(2);
    ei->getPhysicsHandler()->setGravity({ 0, 1000 });

    initUserInputHandlers();

    while (window.isOpen()) {
        ei->onStartFrame();
        window.clear();

        sf::Event event;
        while (window.pollEvent(event)) {
            ei->getUserInput()->handleEvent(event);
        }

        auto dt = ei->getSimDt();
        ei->getPhysicsHandler()->update(dt);
        auto scene = ei->getScene();
        scene->updateRec(dt);

        window.draw(*scene);
        window.draw(*ei->getBodyPullHandler());
        window.display();
    }

    return EXIT_SUCCESS;
}
