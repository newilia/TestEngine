#pragma once

#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/SubscriptionsHolderBase.h"
#include "LaunchProfiles/LaunchProfileBase.h"

namespace Billiard {

	class BilliardGameLaunchProfile : public LaunchProfileBase,
	                                  public Engine::EventHandlerBase,
	                                  public SubscriptionsHolderBase
	{
	public:
		~BilliardGameLaunchProfile() override = default;
		void Setup() override;
		void OnEvent(const sf::Event& event) override;

	private:
	};

} // namespace Billiard
