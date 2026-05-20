#include "Engine/Editor/Commands/SetNodeWorldPositionCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

namespace Engine::EditorCommands {

	SetNodeWorldPositionCommand::SetNodeWorldPositionCommand(
	    std::shared_ptr<SceneNode> node, sf::Vector2f oldWorldPos, sf::Vector2f newWorldPos)
	    : _node(std::move(node)), _oldWorldPos(oldWorldPos), _newWorldPos(newWorldPos) {}

	bool SetNodeWorldPositionCommand::Execute() {
		auto node = _node.lock();
		if (!node) {
			return false;
		}
		Utils::SetLocalPosToWorld(node, _newWorldPos);
		return true;
	}

	void SetNodeWorldPositionCommand::Undo() {
		auto node = _node.lock();
		if (!node) {
			return;
		}
		Utils::SetLocalPosToWorld(node, _oldWorldPos);
	}

} // namespace Engine::EditorCommands
