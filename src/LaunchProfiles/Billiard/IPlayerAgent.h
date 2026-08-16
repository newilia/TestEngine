#pragma once

#include "BilliardRulesSnapshot.h"
#include "BilliardTableSnapshot.h"
#include "BilliardTurnIntent.h"

#include <SFML/System/Time.hpp>

#include <optional>

namespace sf {
	class Event;
}

namespace Billiard {

	class IPlayerAgent
	{
	public:
		virtual ~IPlayerAgent() = default;

		virtual void OnTurnStarted(const TableSnapshot& table, const RulesSnapshot& rules) = 0;
		virtual void OnTurnUpdate(const sf::Time& deltaTime) = 0;
		virtual void OnTurnEnded() = 0;
		virtual void OnEvent(const sf::Event& event) = 0;

		[[nodiscard]] virtual bool HasPendingIntent() const = 0;
		virtual std::optional<TurnIntent> ConsumeIntent() = 0;
		[[nodiscard]] virtual bool WantsInput() const = 0;
	};

} // namespace Billiard
