#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <vector>

namespace Billiard {

	struct BallSnapshot
	{
		int ballNumber = 0;
		sf::Vector2f position{};
		sf::Vector2f velocity{};
		float angularSpeed = 0.f;
		sf::Vector3f spinOmega{};
		bool isOnTable = true;
	};

	struct TableSnapshot
	{
		std::vector<BallSnapshot> balls;
	};

} // namespace Billiard
