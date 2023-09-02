#pragma once
#include <memory>
#include "Scene.h"

class SceneBuilder {
public:
	SceneBuilder() = default;
	std::shared_ptr<Scene> buildScene();
private:
};