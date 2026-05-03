#pragma once
#include "Engine/App/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "Environments/EnvironmentBase.h"

#include <memory>

class TestEnvironment : public EnvironmentBase, public Engine::EventHandlerBase
{
public:
	~TestEnvironment() override = default;
	void Setup() override;
	void OnEvent(const sf::Event& event) override;

private:
	std::shared_ptr<Scene> BuildScene();
};
