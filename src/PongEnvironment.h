#pragma once
#include <memory>

#include "Scene.h"
#include "PongPlatform.h"

class PongEnvironment {
public:
	void setup();
private:
	std::shared_ptr<Scene> buildScene();
	void configureInput();
};