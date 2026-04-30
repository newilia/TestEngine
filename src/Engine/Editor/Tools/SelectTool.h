#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Tools/IEditorTool.h"

#include <functional>
#include <memory>

/// Sets hierarchy selection from scene picks (mouse / primary touch).
class SelectTool final : public IEditorTool
{
public:
	using SelectCallback = std::function<void(std::shared_ptr<SceneNode>)>;

	explicit SelectTool(SelectCallback onSelect);

	[[nodiscard]] bool processEvent(const sf::Event& event) override;

private:
	SelectCallback _onSelect;
};
