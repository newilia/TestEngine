#include "Engine/Editor/Commands/EditorSceneHelpers.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"

namespace Engine::EditorCommands {

	bool IsUnderActiveScene(const std::shared_ptr<SceneNode>& node) {
		const auto active = MainContext::GetInstance().GetScene();
		if (!active || !node) {
			return false;
		}
		const auto activeRoot = active->GetRoot();
		if (!activeRoot) {
			return false;
		}
		auto current = node;
		while (auto parent = current->GetParent()) {
			current = std::move(parent);
		}
		return current.get() == activeRoot.get();
	}

} // namespace Engine::EditorCommands
