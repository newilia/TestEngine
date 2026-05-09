#include "PeriodicTaskExecutor.h"

#include <SFML/System/Time.hpp>

PeriodicTaskExecutor::PeriodicTaskExecutor(
    std::function<sf::Time()> getPeriod, std::function<void(const sf::Time& dt)> task) {
	_getPeriod = std::move(getPeriod);
	_task = std::move(task);
}

bool PeriodicTaskExecutor::Update(sf::Time dt) {
	_accumulated += dt;
	if (_accumulated >= _getPeriod()) {
		_task(_accumulated);
		_accumulated = sf::Time();
		return true;
	}
	return false;
}
