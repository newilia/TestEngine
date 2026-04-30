#include "EditorToolManager.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Tools/MoveTool.h"
#include "Engine/Editor/Tools/PullTool.h"
#include "Engine/Editor/Tools/SelectTool.h"
#include "Engine/Editor/Tools/TapTool.h"

#include <fmt/format.h>

#include <algorithm>
#include <utility>

std::optional<int> EditorToolManager::TryToolIndexFromDigitKey(const sf::Keyboard::Key key) {
	switch (key) {
	case sf::Keyboard::Key::Num1:
		return 0;
	case sf::Keyboard::Key::Num2:
		return 1;
	case sf::Keyboard::Key::Num3:
		return 2;
	case sf::Keyboard::Key::Num4:
		return 3;
	case sf::Keyboard::Key::Num5:
		return 4;
	case sf::Keyboard::Key::Num6:
		return 5;
	case sf::Keyboard::Key::Num7:
		return 6;
	case sf::Keyboard::Key::Num8:
		return 7;
	case sf::Keyboard::Key::Num9:
		return 8;
	case sf::Keyboard::Key::Num0:
		return 9;
	default:
		return std::nullopt;
	}
}

std::string EditorToolManager::FormatToolPaletteLabel(const int toolIndex, const char* const displayName) {
	static constexpr char kDigits[] = "1234567890";
	if (toolIndex >= 0 && toolIndex < static_cast<int>(sizeof(kDigits)) - 1) {
		return fmt::format("[{}] {}", kDigits[toolIndex], displayName);
	}
	return std::string(displayName);
}

bool EditorToolManager::TryActivateToolViaDigitKey(const sf::Keyboard::Key key) {
	const std::optional<int> slot = TryToolIndexFromDigitKey(key);
	if (!slot || *slot < 0 || *slot >= kToolCount) {
		return false;
	}
	SetActiveToolIndex(*slot);
	return true;
}

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
