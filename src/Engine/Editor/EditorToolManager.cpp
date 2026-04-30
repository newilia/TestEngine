#include "EditorToolManager.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Tools/MoveTool.h"
#include "Engine/Editor/Tools/PullTool.h"
#include "Engine/Editor/Tools/SelectTool.h"
#include "Engine/Editor/Tools/TapTool.h"

#include <algorithm>
#include <utility>

EditorToolManager::EditorToolManager() {
	const auto setHierarchySelection = [](std::shared_ptr<SceneNode> node) {
		Engine::Editor::GetInstance().SetSelectedNode(std::move(node));
	};
	auto select = std::make_unique<SelectTool>(setHierarchySelection);
	auto pull = std::make_unique<PullTool>(setHierarchySelection);
	_pullTool = pull.get();

	_tools[0] = std::make_unique<TapTool>();
	_tools[1] = std::move(select);
	_tools[2] = std::move(pull);
	_tools[3] = std::make_unique<MoveTool>(setHierarchySelection);
}

void EditorToolManager::SetActiveToolIndex(int index) {
	_activeToolIndex = std::clamp(index, 0, kToolCount - 1);
}

bool EditorToolManager::ProcessEvent(const sf::Event& event) {
	if (_activeToolIndex < 0 || _activeToolIndex >= kToolCount || !_tools[_activeToolIndex]) {
		return false;
	}
	return _tools[_activeToolIndex]->processEvent(event);
}

void EditorToolManager::OnPresent(const sf::Time& dt) {
	if (_activeToolIndex < 0 || _activeToolIndex >= kToolCount || !_tools[_activeToolIndex]) {
		return;
	}
	_tools[_activeToolIndex]->onPresent(dt);
}

void EditorToolManager::BindPullArrow(std::shared_ptr<VectorArrowVisual> arrow) {
	if (_pullTool) {
		_pullTool->SetArrowVisual(std::move(arrow));
	}
}
