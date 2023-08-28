#pragma once
#include <SFML/System/Vector2.hpp>
#include "ComponentBase.h"
#include "Utils.h"

class UserPullComponent : public ComponentBase {
public:
	UserPullComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	~UserPullComponent() override = default;

	sf::Vector2f getPullVector() const;

	sf::Vector2f mSourcePoint; // local coordinate of body's pulling point
	sf::Vector2f mDestPoint; // global coordinate of pointer
	float mPullingStrength = 100000.f;

	enum class PullMode {
		HARD,
		SOFT
	} mMode = PullMode::SOFT;
};
