#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

#include <limits>
#include <memory>

class RigidBodyBehaviour;

class AttractiveBehaviour : public Behaviour
{
	META_CLASS()
public:
	void OnInit() override;
	void OnDeinit() override;
	void OnUpdate(const sf::Time& dt) override;
	/// @property
	bool _isEnabled = true;
	/// @property(minValue=-100.0, maxValue=100.0, step=0.5, dragSpeed=0.1, tooltip="Negative: attraction; positive:
	/// repulsion; magnitude is nonlinear in |value|.")
	float _attraction = 0.f;

private:
	std::weak_ptr<RigidBodyBehaviour> _rigidBody;
	std::weak_ptr<AttractiveBehaviour> _self;
};
