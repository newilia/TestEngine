#pragma once
#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "LaunchProfiles/LaunchProfileBase.h"

#include <memory>

class TestLaunchProfile : public LaunchProfileBase, public Engine::EventHandlerBase
{
public:
	~TestLaunchProfile() override = default;
	void Setup() override;
	void OnEvent(const sf::Event& event) override;

private:
	std::shared_ptr<Scene> BuildScene();
};
