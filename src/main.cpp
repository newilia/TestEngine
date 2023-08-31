#include <iostream>
#include <SFML/Graphics.hpp>

#include "BodyPullHandler.h"
#include "common.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "UserInput.h"

namespace
{
    const sf::Vector2u wndSize(800, 600);
}
float SIM_SPEED = 1.f;
bool SIM_PAUSE = false;


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
int main()
{
    sf::RenderWindow window(sf::VideoMode(wndSize.x, wndSize.y), "Pong", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60u);

    auto scene = make_shared<Scene>();
    SceneLoader loader(scene.get());
    loader.loadScene();

    UserInput userInput;

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
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            userInput.handleEvent(event);
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
