#pragma once

#include "Engine/Editor/EditorNodePick.h"
#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

class SceneNode;

/// LMB drag: moves picked node in window space, clears rigid body velocity while dragging.
class MoveTool final : public IEditorTool
{
public:
	explicit MoveTool(EditorNodePick::SelectCallback onSelect = nullptr);

	bool ProcessEvent(const sf::Event& event) override;
	void DrawToolParametersUi() override;

private:
	EditorNodePick::SelectCallback _onSelect;
	std::weak_ptr<SceneNode> _grabbedNode;
	sf::Vector2f _grabOffset{};
	bool _dragging = false;
	bool _wasBodyFixed = false;
};
