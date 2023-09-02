#include <iostream>
#include <SFML/Graphics.hpp>

#include "BodyPullHandler.h"
#include "common.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "UserInput.h"

namespace
{
    const sf::Vector2u wndSize(800, 600);
}
float SIM_SPEED = 1.f;
bool SIM_PAUSE = false;

int main()
{
    sf::RenderWindow window(sf::VideoMode(wndSize.x, wndSize.y), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60u);
    
    SceneBuilder builder;
    auto scene = builder.buildScene();

    auto userInput = UserInput::getInstance();
    
    userInput->attachCustomHandler(createDelegate<sf::Event>([&scene, &builder](sf::Event event) {
        if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::R) {
            scene = builder.buildScene();
        }
	}));

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            userInput->handleEvent(event);
        }

        auto dt = SIM_PAUSE ? sf::Time() : clock.getElapsedTime();
        dt *= SIM_SPEED;
        clock.restart();


        scene->updateRec(dt);
        window.clear();
        window.draw(*scene);
        window.draw(*BodyPullHandler::getInstance());
        window.display();

        PhysicsHandler::getInstance()->update(dt);

    }

    return EXIT_SUCCESS;
}
