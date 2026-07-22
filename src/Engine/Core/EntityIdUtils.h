#pragma once

#include "Engine/Core/EntityId.h"

#include <unordered_set>

namespace Engine {

	[[nodiscard]] EntityId RandomNonZeroEntityId();
	void EnsureUniqueEntityId(EntityId& id, std::unordered_set<EntityId>& claimed);

} // namespace Engine
