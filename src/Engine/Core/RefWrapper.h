#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"

#include <memory>
#include <type_traits>

/// Stable reference to a `SceneNode` or `EntityOnNode` in a scene by `EntityId`.
/// Resolve only through `Resolve(const Scene&)` (no global scene). Caches a `weak_ptr` for repeated lookups.
template <typename T>
class RefWrapper
{
public:
	[[nodiscard]] Engine::EntityId GetId() const {
		return _id;
	}

	void SetId(Engine::EntityId id) {
		_id = id;
		_cached.reset();
	}

	void Clear() {
		_id = Engine::kInvalidEntityId;
		_cached.reset();
	}

	[[nodiscard]] explicit operator bool() const {
		return _id != Engine::kInvalidEntityId;
	}

	[[nodiscard]] std::shared_ptr<T> Get(const Scene& scene) const {
		constexpr bool isNode = std::is_same_v<T, SceneNode>;
		constexpr bool isEntity = std::is_base_of_v<EntityOnNode, T> && !isNode;
		static_assert(isNode || isEntity, "RefWrapper<T>: T must be SceneNode or a subclass of EntityOnNode");

		if (_id == Engine::kInvalidEntityId) {
			_cached.reset();
			return nullptr;
		}
		if (auto locked = _cached.lock()) {
			if (locked->GetEntityId() == _id) {
				return locked;
			}
		}
		if constexpr (isNode) {
			auto n = scene.FindNodeByEntityId(_id);
			_cached = n;
			return n;
		}
		else {
			auto e = scene.FindEntityByEntityId(_id);
			auto t = e ? std::dynamic_pointer_cast<T>(e) : nullptr;
			_cached = t;
			return t;
		}
	}

	[[nodiscard]] std::shared_ptr<T> Get() const {
		if (auto scene = Engine::MainContext::GetInstance().GetScene()) {
			return Get(*scene);
		}
		return nullptr;
	}

private:
	Engine::EntityId _id = Engine::kInvalidEntityId;
	mutable std::weak_ptr<T> _cached{};
};
