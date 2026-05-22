#include "EditorProfile.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <SFML/Graphics.hpp>

#include <cstdlib>
#include <memory>

void EditorProfile::Setup() {
	auto& mainContext = Engine::MainContext::GetInstance();
	const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode({1920, 1080}), "Test Engine");
	if (!mainWindow) {
		std::exit(EXIT_FAILURE);
	}
	Utils::MaximizeWindow(*mainWindow);
	mainContext.SetScene(std::make_shared<Scene>());
}
