#include <SFML/Graphics.hpp>

#include "BodyPullHandler.h"
#include "common.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "UserInput.h"
#include "UserInputConfiguration.h"
#include <fmt/format.h>

namespace
{
    const sf::Vector2u wndSize(800, 600);
}

int main() {
    auto gi = GlobalInterface::getInstance();
    sf::RenderWindow window(sf::VideoMode(wndSize.x, wndSize.y), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    gi->setMainWindow(window);
    gi->setScene(gi->getSceneBuilder()->buildScene());

    initUserInputHandlers();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            gi->getUserInput()->handleEvent(event);
        }

        auto dt = gi->getFrameDtAndReset();
        gi->getPhysicsHandler()->update(dt);
        auto scene = gi->getScene();
        scene->updateRec(dt);

        window.clear();
        window.draw(*scene);
        window.draw(*gi->getBodyPullHandler());
        window.display();
        
    }

    return EXIT_SUCCESS;
}
