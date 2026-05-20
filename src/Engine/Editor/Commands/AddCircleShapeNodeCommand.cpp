#include "Engine/Editor/Commands/AddCircleShapeNodeCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Editor/Commands/EditorShapeNodeBuilders.h"
#include "Engine/Editor/Editor.h"

namespace Engine::EditorCommands {

	AddCircleShapeNodeCommand::AddCircleShapeNodeCommand(std::shared_ptr<SceneNode> parent, const std::size_t index,
	    const sf::Vector2f centerWorld, const float radius, const bool attachPhysics,
	    const Utils::HsvShapeColors colors)
	    : _parent(std::move(parent)), _index(index), _centerWorld(centerWorld), _radius(radius),
	      _attachPhysics(attachPhysics), _colors(colors) {}

	bool AddCircleShapeNodeCommand::Execute() {
		auto parent = _parent.lock();
		if (!parent) {
			return false;
		}
		auto node = BuildCircleShapeNode(_centerWorld, _radius, _attachPhysics, _colors);
		parent->AddChildAt(node, _index);
		if (IsUnderActiveScene(node)) {
			node->NotifyLifecycleInitRecursive();
		}
		_node = node;
		Engine::Editor::GetInstance().SetSelectedNode(node);
		return true;
	}

	void AddCircleShapeNodeCommand::Undo() {
		auto node = _node.lock();
		auto parent = _parent.lock();
		if (!node || !parent) {
			return;
		}
		parent->RemoveChild(node.get());
	}

} // namespace Engine::EditorCommands
