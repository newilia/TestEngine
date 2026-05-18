#include "Engine/Editor/EditorNodePick.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"

#include <SFML/Window/Keyboard.hpp>

namespace EditorNodePick {

	bool IsMultiSelectModifierPressed() {
		return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
		       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
	}

	sf::Vector2f MapWindowPixelToWorld(const sf::Vector2i pixel) {
		if (auto* window = Engine::MainContext::GetInstance().GetMainWindow()) {
			return Utils::MapWindowPixelToWorld(*window, pixel);
		}
		return {};
	}

	void ApplyHierarchyPickAtWorld(const sf::Vector2f world, const bool isCtrlPressed, const SelectCallback& onSelect) {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = scene ? scene->FindTopMostNodeAtPoint(world) : nullptr;
		if (isCtrlPressed && picked) {
			Engine::Editor::GetInstance().ToggleSelectedNode(std::move(picked));
		}
		else if (onSelect) {
			onSelect(std::move(picked));
		}
	}

} // namespace EditorNodePick
