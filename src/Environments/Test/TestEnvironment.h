#pragma once
#include "Engine/Core/Scene.h"
#include "Environments/EnvironmentBase.h"

#include <memory>

class TestEnvironment : public EnvironmentBase
{
public:
	~TestEnvironment() override = default;
	void Setup() override;

private:
	std::shared_ptr<Scene> BuildScene();
	void ConfigureInput();
};
