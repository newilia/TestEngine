#pragma once
#include <memory>

#include "Scene.h"

class TestEnvironment {
public:
	static void setup();
private:
	static std::shared_ptr<Scene> buildScene();
	static void configureInput();
};