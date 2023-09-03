#pragma once
#include <SFML/System/Vector2.hpp>
#include "ComponentBase.h"
#include "Utils.h"

class UserPullComponent : public ComponentBase {
public:
	UserPullComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	~UserPullComponent() override = default;

	sf::Vector2f getPullVector() const;

	sf::Vector2f mLocalSourcePoint;
	sf::Vector2f mGlobalDestPoint;
	float mPullingStrength = 100000.f;

	enum class PullMode {
		POSITION,
		FORCE,
		VELOCITY,
	} mMode = PullMode::FORCE;
};
