#include "Engine/Editor/SceneClipboard.h"

#include "Engine/Core/SceneNode.h"

namespace Engine {

	bool SceneClipboard::CopyNode(const std::shared_ptr<SceneNode>& node) {
		auto prototype = CloneSceneNode(node);
		if (!prototype) {
			return false;
		}
		_payload = NodePayload{.prototype = std::move(prototype)};
		return true;
	}

	bool SceneClipboard::CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot) {
		auto prototype = CloneEntity(entity);
		if (!prototype) {
			return false;
		}
		_payload = EntityPayload{.slot = slot, .prototype = std::move(prototype)};
		return true;
	}

	bool SceneClipboard::HasNode() const {
		return std::holds_alternative<NodePayload>(_payload);
	}

	bool SceneClipboard::HasEntity() const {
		return std::holds_alternative<EntityPayload>(_payload);
	}

	EntitySlot SceneClipboard::GetEntitySlot() const {
		if (const auto* payload = std::get_if<EntityPayload>(&_payload)) {
			return payload->slot;
		}
		return EntitySlot::Behaviour;
	}

	std::shared_ptr<SceneNode> SceneClipboard::InstantiateNode() const {
		if (const auto* payload = std::get_if<NodePayload>(&_payload)) {
			return CloneSceneNode(payload->prototype);
		}
		return nullptr;
	}

	std::shared_ptr<EntityOnNode> SceneClipboard::InstantiateEntity() const {
		if (const auto* payload = std::get_if<EntityPayload>(&_payload)) {
			return CloneEntity(payload->prototype);
		}
		return nullptr;
	}

} // namespace Engine
