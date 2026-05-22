#pragma once

#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "LaunchProfiles/LaunchProfileBase.h"

namespace Demo1 {

	class Demo1LaunchProfile : public LaunchProfileBase, public Engine::EventHandlerBase
	{
	public:
		~Demo1LaunchProfile() override = default;
		void Setup() override;
		void OnEvent(const sf::Event& event) override;

	private:
		std::shared_ptr<Scene> BuildScene();
	};

} // namespace Demo1
