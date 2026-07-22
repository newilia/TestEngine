#include "Engine/Core/EntityIdUtils.h"

#include <limits>
#include <random>

namespace Engine {

	EntityId RandomNonZeroEntityId() {
		thread_local std::mt19937 rng{std::random_device{}()};
		std::uniform_int_distribution<std::uint32_t> dist(1u, (std::numeric_limits<std::uint32_t>::max)());
		return dist(rng);
	}

	void EnsureUniqueEntityId(EntityId& id, std::unordered_set<EntityId>& claimed) {
		if (id != kInvalidEntityId && !claimed.contains(id)) {
			claimed.insert(id);
			return;
		}
		do {
			id = RandomNonZeroEntityId();
		}
		while (claimed.contains(id));
		claimed.insert(id);
	}

} // namespace Engine
