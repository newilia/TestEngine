#include "PeriodicTaskExecutor.h"

#include <SFML/System/Time.hpp>

PeriodicTaskExecutor::PeriodicTaskExecutor(std::function<sf::Time()> getPeriod,
                                           std::function<void(const sf::Time& dt)> task) {
	_getPeriod = std::move(getPeriod);
	_task = std::move(task);
}

bool PeriodicTaskExecutor::Run(sf::Time dt) {
	_accumulated += dt;
	const auto period = _getPeriod();
	if (_accumulated >= period) {
		_accumulated -= period;
		if (_accumulated >= period) {
			_accumulated = sf::Time();
		}
		_task(period);
		return true;
	}
	return false;
}
