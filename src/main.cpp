#include <SFML/Graphics.hpp>

#define DEBUG true
#include "BodyPullHandler.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "UserInput.h"
#include "UserInputConfiguration.h"
#include <fmt/format.h>

namespace {
    const sf::Vector2u wndSize(800, 600);
}

int main() {
    auto ei = EI();
    sf::RenderWindow window(sf::VideoMode(wndSize.x, wndSize.y), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60u);
    ei->setMainWindow(window);
    ei->setScene(ei->getSceneBuilder()->buildScene());

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
