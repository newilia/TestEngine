#include <SFML/Graphics.hpp>
#include "UserInput.h"
#include <fmt/format.h>

#include "EngineInterface.h"
#include "Pong/PongEnvironment.h"

#ifdef _CONSOLE
int main() {
#else
#define NOMINMAX 
#include <windows.h>
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif
    _set_error_mode(_OUT_TO_MSGBOX);
    //TestEnvironment env;
    PongEnvironment env;
    env.setup();

    auto ei = EI();
    while (true) {
        auto window = EI()->getMainWindow();
        if (!window) {
            continue;
        }
        if (!window->isOpen()) {
            break;
        }
        auto scene = ei->getScene();
        if (!scene) {
            continue;
        }

        ei->onStartFrame();
        window->clear();

        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return EXIT_SUCCESS;
            }
            ei->getUserInput()->handleEvent(event);
        }

        auto dt = ei->getSimDt();
        if (!ei->isSimPaused()) {
            ei->getPhysicsHandler()->update(dt);
        }
        if (dt.asSeconds() > 0.1f) {
            dt = sf::seconds(0.1f);
        }
        scene->updateRec(dt);

        window->draw(*scene);
        window->draw(*ei->getBodyPullHandler());
        window->display();
    }

    return EXIT_SUCCESS;
}
