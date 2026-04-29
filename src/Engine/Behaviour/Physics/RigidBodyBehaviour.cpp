#include "RigidBodyBehaviour.h"

#include "RigidBodyBehaviour_gen.hpp"

#include <limits>

void RigidBodyBehaviour::SetImmovable() {
	_mass = std::numeric_limits<float>::infinity();
}

bool RigidBodyBehaviour::IsImmovable() const {
	return _mass == std::numeric_limits<float>::infinity();
}
