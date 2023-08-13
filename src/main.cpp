#include <iostream>
#include <SFML/Graphics.hpp>

#include "Ball.h"
#include "common.h"
#include "Physics/PhysicsHandler.h"
#include "PongPlatform.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "Utils.h"

namespace
{
    const sf::Vector2u wndSize(800, 600);
    float dtScale = 1.f;
    bool pause = false;
}


//void handleCollisions(PongPlatform& platform, Ball& ball, sf::CircleShape& collisionPoint) {
//    const auto& platformBb = platform.getShape().getGlobalBounds();
//    const auto& ballBb = ball.getShape().getGlobalBounds();
//	if (!platformBb.intersects(ballBb)) {
//        return;
//	}
//    const sf::Vector2f& ballCenter = ball.getShape().getPosition();
//    sf::Vector2f nearestPlatformPoint;
//
//    if (ballCenter.x < platformBb.left) {
//        nearestPlatformPoint.x = platformBb.left;
//        nearestPlatformPoint.y = std::clamp(ballCenter.y, platformBb.top, platformBb.top + platformBb.height);
//    }
//    else if (ballCenter.x > platformBb.left + platformBb.width) {
//        nearestPlatformPoint.x = platformBb.left + platformBb.width;
//        nearestPlatformPoint.y = std::clamp(ballCenter.y, platformBb.top, platformBb.top + platformBb.height);
//    }
//    else {
//        nearestPlatformPoint.x = ballCenter.x;
//        nearestPlatformPoint.y = platformBb.top;
//    }
//    //collisionPoint.setPosition(nearestPlatformPoint);
//    auto ballToPlatformVector = nearestPlatformPoint - ballCenter;
//    if (utils::calcLength(ballToPlatformVector) < ball.getShape().getRadius()) {
//        auto reflSpeed = utils::reflect(ball.getSpeed(), ballToPlatformVector);
//        ball.setSpeed(reflSpeed);
//    }
//}

void handleGameSpeed(const sf::Event& event) {
	if (event.type == sf::Event::KeyPressed) {
		switch (event.key.code) {
		case sf::Keyboard::Equal:
            dtScale *= 2;
            break;
		case sf::Keyboard::Hyphen:
            dtScale *= 0.5f;
            break;
		case sf::Keyboard::Num0:
            pause = !pause;
            break;
        default: break;
		}
	}
}

int main()
{

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(wndSize.x, wndSize.y), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60u);

    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf")) {
        return EXIT_FAILURE;
    }
        
    auto scene = make_shared<Scene>();
    SceneLoader loader(scene.get());
    loader.loadScene();

    /*PongPlatform platform;
    {
        constexpr float airFriction = 30;
        constexpr float playerAccel = 50000.f;
        constexpr float platformMaxSpeed = 3000.f;
        const sf::Vector2f platformSize(200, 50);
        const sf::Vector2f initPos((wndSize.x - platformSize.x) / 2, wndSize.y - 75.f);
        const sf::FloatRect area({ 0, 0 }, static_cast<const sf::Vector2f>(wndSize));

        platform.getShape().setPosition(initPos);
        platform.getShape().setSize(platformSize);
        platform.getShape().setFillColor(sf::Color::White);
        platform.setMoveArea(area);
        platform.setAirFriction(airFriction);
        platform.setMaxSpeed(platformMaxSpeed);
        platform.setPlayerAccelMagnitude(playerAccel);
    }

    Ball ball;
    {
        const sf::FloatRect area({ 0, 0 }, static_cast<const sf::Vector2f>(wndSize));
        ball.setMoveArea(area);
        ball.setSpeed({ 200, -500 });
        const sf::Vector2f initPos(wndSize.x / 2, wndSize.y / 2);
        ball.getShape().setPosition(initPos);
        ball.setRadius(30);
    }

    sf::CircleShape collisionPoint(4);
    collisionPoint.setFillColor(sf::Color::Red);
    collisionPoint.setOrigin(2, 2);
    */

    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Window closed or escape key pressed: exit
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }
            handleGameSpeed(event);
        }

        auto dt = pause ? sf::Time() : clock.getElapsedTime();
        dt *= dtScale;
        clock.restart();

        scene->updateRec(dt);
        PhysicsHandler::getInstance()->update(dt);

        window.clear();
        window.draw(*scene);
        window.display();


    }

    return EXIT_SUCCESS;
}
