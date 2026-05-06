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

	/* TODO incapsulation */
	/// @property(minValue=0.f, maxValue=100000.f)
	float _speedFactor = 50.f;
	/// @property
	sf::Vector2f _velLimit = {15000.f, 1000.f};

private:
	sf::Vector2f _defaultPos{};
	sf::Vector2f _targetPos{};
	std::weak_ptr<SceneNode> _movementBounds;
};
