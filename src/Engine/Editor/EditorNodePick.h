#pragma once

#include "Engine/Core/SceneNode.h"

#include <SFML/System/Vector2.hpp>

#include <functional>
#include <memory>

/// Shared scene pick for editor tools (click / Ctrl+toggle).
namespace EditorNodePick {

	using SelectCallback = std::function<void(std::shared_ptr<SceneNode>)>;

	bool IsMultiSelectModifierPressed();
	sf::Vector2f MapWindowPixelToWorld(sf::Vector2i pixel);
	void ApplyHierarchyPickAtWorld(sf::Vector2f world, bool isCtrlPressed, const SelectCallback& onSelect);

} // namespace EditorNodePick
