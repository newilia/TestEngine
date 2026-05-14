#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/Scene.h"

#include <cstdint>
#include <memory>
#include <type_traits>

/// Stable reference to a `SceneNode` or `EntityOnNode` in a scene by `SceneObjectId`.
/// Resolve only through `Resolve(const Scene&)` (no global scene). Caches a `weak_ptr` for repeated lookups.
template <typename T>
class RefWrapper
{
public:
	[[nodiscard]] Engine::SceneObjectId GetId() const {
		return _id;
	}

	void SetId(Engine::SceneObjectId id) {
		_id = id;
		_cached.reset();
	}

	void Clear() {
		_id = Engine::kInvalidSceneObjectId;
		_cached.reset();
	}

	[[nodiscard]] explicit operator bool() const {
		return _id != Engine::kInvalidSceneObjectId;
	}

	[[nodiscard]] std::shared_ptr<T> Resolve(const Scene& scene) const {
		// Checked here (not at class scope): `RefWrapper<Derived>` members inside `Derived` instantiate
		// `RefWrapper<Derived>` while `Derived` is still incomplete, so `std::is_base_of` would be unreliable.
		constexpr bool isNode = std::is_same_v<T, SceneNode>;
		constexpr bool isEntity = std::is_base_of_v<EntityOnNode, T> && !isNode;
		static_assert(isNode || isEntity, "RefWrapper<T>: T must be SceneNode or a subclass of EntityOnNode");

		if (_id == Engine::kInvalidSceneObjectId) {
			_cached.reset();
			return nullptr;
		}
		if (auto locked = _cached.lock()) {
			if (locked->GetSceneObjectId() == _id) {
				return locked;
			}
		}
		if constexpr (isNode) {
			auto n = scene.FindNodeByObjectId(_id);
			_cached = n;
			return n;
		}
		else {
			auto e = scene.FindEntityByObjectId(_id);
			auto t = e ? std::dynamic_pointer_cast<T>(e) : nullptr;
			_cached = t;
			return t;
		}
	}

private:
	Engine::SceneObjectId _id = Engine::kInvalidSceneObjectId;
	mutable std::weak_ptr<T> _cached{};
};
