#pragma once
#include <SFML/System/Time.hpp>

#include <functional>

class PeriodicTaskExecutor
{
public:
	explicit PeriodicTaskExecutor(std::function<sf::Time()> getPeriod, std::function<void(const sf::Time& dt)> task);
	bool Update(sf::Time dt);

private:
	std::function<sf::Time()> _getPeriod;
	std::function<void(const sf::Time& dt)> _task;
	sf::Time _accumulated{};
};
