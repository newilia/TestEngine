#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"
#include "Engine/Editor/Tools/SelectTool.h"

#include <SFML/System/Vector2.hpp>

#include <optional>
#include <string>

namespace sf {
	class RenderWindow;
}

/// LMB / primary touch drag: corner to opposite corner, creates a RectangleShapeVisual on a new child node.
class RectangleTool final : public IEditorTool
{
public:
	explicit RectangleTool(SelectTool::SelectCallback onSelect = nullptr);

	bool ProcessEvent(const sf::Event& event) override;
	void DrawOverlay(sf::RenderWindow& window) override;
	void DrawToolParametersUi() override;
	[[nodiscard]] std::optional<std::string> TryGetCursorOverlayText() const override;

private:
	void BeginStroke(const sf::Vector2f& world);
	void EndStroke();
	void FinalizeStroke();

	SelectTool::SelectCallback _onSelect;
	bool _isDrawing = false;
	sf::Vector2f _startWorld{};
	sf::Vector2f _cursorWorld{};
	bool _isAttachPhysicsBody = true;
};
