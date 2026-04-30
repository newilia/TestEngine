#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"
#include "Engine/Editor/Tools/SelectTool.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

class SceneNode;

/// LMB drag: moves picked node in window space, clears rigid body velocity while dragging.
class MoveTool final : public IEditorTool
{
public:
	explicit MoveTool(SelectTool::SelectCallback onSelect);

	[[nodiscard]] bool processEvent(const sf::Event& event) override;

private:
	SelectTool::SelectCallback _onSelect;
	std::weak_ptr<SceneNode> _grabbed;
	sf::Vector2f _grabOffset{};
	bool _dragging = false;
};
