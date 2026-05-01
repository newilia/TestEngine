#include "RigidBodyBehaviour.h"

#include "RigidBodyBehaviour.generated.hpp"

#include <limits>

void RigidBodyBehaviour::SetImmovable() {
	_mass = std::numeric_limits<float>::infinity();
}

bool RigidBodyBehaviour::IsImmovable() const {
	return _mass == std::numeric_limits<float>::infinity();
}
