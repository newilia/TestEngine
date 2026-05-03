#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"
#include "Engine/Editor/Tools/SelectTool.h"

#include <SFML/System/Vector2.hpp>

#include <optional>
#include <vector>

namespace sf {
	class RenderWindow;
}

/// LMB / primary touch drag: samples vertices (min 5 px apart on screen), creates a ConvexShapeVisual on a new child node.
class PolygonTool final : public IEditorTool
{
public:
	explicit PolygonTool(SelectTool::SelectCallback onSelect);

	bool processEvent(const sf::Event& event) override;
	void drawOverlay(sf::RenderWindow& window) override;
	void drawToolParametersUi() override;

private:
	void beginStroke(const sf::Vector2f& world, const sf::Vector2i& pixel);
	void endStroke();
	void tryAppendSample(const sf::Vector2i& pixel, const sf::Vector2f& world);
	void finalizeStroke();

	SelectTool::SelectCallback _onSelect;
	bool _isDrawing = false;
	std::vector<sf::Vector2f> _worldSamples;
	std::optional<sf::Vector2i> _lastSamplePixel;
	sf::Vector2f _cursorWorld{};
	bool _isAttachPhysicsBody = true;
};
