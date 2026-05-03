#pragma once

#include "Engine/App/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "Environments/EnvironmentBase.h"

namespace Demo1 {

	class Env : public EnvironmentBase, public Engine::EventHandlerBase
	{
	public:
		~Env() override = default;
		void Setup() override;
		void OnEvent(const sf::Event& event) override;

	private:
		std::shared_ptr<Scene> BuildScene();
	};

} // namespace Demo1
