#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

#include <limits>
#include <memory>

class PhysicsBodyBehaviour;

class AttractiveBehaviour : public Behaviour, public std::enable_shared_from_this<AttractiveBehaviour>
{
	META_CLASS()
public:
	void OnInit() override;
	void OnDeinit() override;
	void OnUpdate(const sf::Time& dt) override;

	bool IsEnabled() const;
	float GetAttraction() const;
	void SetAttraction(float value);

private:
	/// @property
	bool _isEnabled = true;
	/// @property
	float _attraction = 0.f;

private:
	std::weak_ptr<PhysicsBodyBehaviour> _rigidBody;
};
