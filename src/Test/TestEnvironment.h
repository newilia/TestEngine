#pragma once
#include "Engine/Scene.h"

#include <memory>

class TestEnvironment
{
public:
	static void setup();

private:
	static std::shared_ptr<Scene> buildScene();
	static void configureInput();
};