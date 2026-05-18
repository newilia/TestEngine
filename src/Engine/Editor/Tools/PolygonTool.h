#pragma once

#include "Engine/Editor/EditorNodePick.h"
#include "Engine/Editor/EditorShapeStrokeGate.h"
#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/System/Vector2.hpp>

#include <optional>
#include <string>
#include <vector>

namespace sf {
	class RenderWindow;
}

/// LMB / primary touch drag: freehand stroke, convex hull becomes ConvexShapeVisual on a new child node.
class PolygonTool final : public IEditorTool
{
public:
	explicit PolygonTool(EditorNodePick::SelectCallback onSelect = nullptr);

	bool ProcessEvent(const sf::Event& event) override;
	void DrawOverlay(sf::RenderWindow& window) override;
	void DrawToolParametersUi() override;

private:
	void BeginStroke(const sf::Vector2f& world, const sf::Vector2i& pixel);
	void TryAppendSample(const sf::Vector2i& pixel, const sf::Vector2f& world);
	void EndStroke();
	void FinalizeStroke();

	EditorNodePick::SelectCallback _onSelect;
	EditorShapeStrokeGate _strokeGate;
	bool _isDrawing = false;
	std::vector<sf::Vector2f> _worldSamples;
	std::optional<sf::Vector2i> _lastSamplePixel;
	sf::Vector2f _cursorWorld{};
	bool _isAttachPhysicsBody = true;
};
