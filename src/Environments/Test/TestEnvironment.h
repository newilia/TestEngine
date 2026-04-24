#pragma once
#include "Engine/Core/Scene.h"

#include <memory>

class TestEnvironment
{
public:
	static void Setup();

private:
	static std::shared_ptr<Scene> BuildScene();
	static void ConfigureInput();
};
