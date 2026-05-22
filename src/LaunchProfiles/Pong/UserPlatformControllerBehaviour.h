#pragma once

#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

namespace sf {
	class Event;
}

class SceneNode;

/// Player paddle: mouse-driven target + velocity toward target.
class UserPlatformControllerBehaviour : public EventHandlerBehaviourBase
{
	META_CLASS()
public:
	void OnInit() override;
	void OnDeinit() override;
	void OnUpdate(const sf::Time& dt) override;
	void OnEvent(const sf::Event& event) override;

	void ResyncSpawnFromNode();
	void SetMovementBounds(std::weak_ptr<SceneNode> movementRegionRect);

	/// @getter(minValue=0.f, maxValue=100000.f)
	float GetSpeedFactor() const;
	/// @setter
	void SetSpeedFactor(float speedFactor);
	/// @getter
	sf::Vector2f GetVelLimit() const;
	/// @setter
	void SetVelLimit(sf::Vector2f velLimit);

private:
	sf::Vector2f _defaultPos{};
	sf::Vector2f _targetPos{};
	std::weak_ptr<SceneNode> _movementBounds;
	float _speedFactor = 50.f;
	sf::Vector2f _velLimit = {15000.f, 1000.f};
};
