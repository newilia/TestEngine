#include "EditorToolManager.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Tools/MoveTool.h"
#include "Engine/Editor/Tools/PolygonTool.h"
#include "Engine/Editor/Tools/PullTool.h"
#include "Engine/Editor/Tools/SelectTool.h"
#include "Engine/Editor/Tools/TapTool.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <utility>

EditorToolManager::EditorToolManager() {
	const auto setHierarchySelection = [](std::shared_ptr<SceneNode> node) {
		Engine::Editor::GetInstance().SetSelectedNode(std::move(node));
	};
	_tools.emplace_back("Tap", std::make_unique<TapTool>());
	_tools.emplace_back("Select", std::make_unique<SelectTool>(setHierarchySelection));
	_tools.emplace_back("Pull", std::make_unique<PullTool>());
	_tools.emplace_back("Move", std::make_unique<MoveTool>(setHierarchySelection));
	_tools.emplace_back("Polygon", std::make_unique<PolygonTool>());
}

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

std::string EditorToolManager::GetToolFormattedName(int index) const {
	static constexpr char kDigits[] = "1234567890";
	if (index >= 0 && index < static_cast<int>(sizeof(kDigits)) - 1) {
		return fmt::format("[{}] {}", kDigits[index], _tools[index].first);
	}
	return "";
}

int EditorToolManager::GetToolCount() const {
	return static_cast<int>(_tools.size());
}

bool EditorToolManager::TryActivateToolViaDigitKey(const sf::Keyboard::Key key) {
	const std::optional<int> slot = TryToolIndexFromDigitKey(key);
	if (!slot || *slot < 0 || *slot >= GetToolCount()) {
		return false;
	}
	SetActiveToolIndex(*slot);
	return true;
}

int EditorToolManager::GetActiveToolIndex() const {
	return _activeToolIndex;
}

void EditorToolManager::SetActiveToolIndex(int index) {
	_activeToolIndex = std::clamp(index, 0, GetToolCount() - 1);
}

bool EditorToolManager::ProcessEvent(const sf::Event& event) {
	if (_activeToolIndex < 0 || _activeToolIndex >= GetToolCount() || !_tools[_activeToolIndex].second) {
		return false;
	}
	return _tools[_activeToolIndex].second->ProcessEvent(event);
}

void EditorToolManager::OnUpdate(const sf::Time& dt) {
	if (_activeToolIndex < 0 || _activeToolIndex >= GetToolCount() || !_tools[_activeToolIndex].second) {
		return;
	}
	_tools[_activeToolIndex].second->Update(dt);
}

void EditorToolManager::DrawOverlay(sf::RenderWindow& window) {
	if (_activeToolIndex < 0 || _activeToolIndex >= GetToolCount() || !_tools[_activeToolIndex].second) {
		return;
	}
	_tools[_activeToolIndex].second->DrawOverlay(window);
}

void EditorToolManager::DrawActiveToolParametersUi() {
	if (_activeToolIndex < 0 || _activeToolIndex >= GetToolCount() || !_tools[_activeToolIndex].second) {
		return;
	}
	_tools[_activeToolIndex].second->DrawToolParametersUi();
}
