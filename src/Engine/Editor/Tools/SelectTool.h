#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

/// Sets hierarchy selection from scene picks (mouse / primary touch).
class SelectTool final : public IEditorTool
{
public:
	using SelectCallback = std::function<void(std::shared_ptr<SceneNode>)>;

	explicit SelectTool(SelectCallback onSelect);

	[[nodiscard]] bool ProcessEvent(const sf::Event& event) override;
	void DrawOverlay(sf::RenderWindow& window) override;
	void DrawToolParametersUi() override;

private:
	enum class MarqueeMode
	{
		kIntersects,
		kContains
	};

	[[nodiscard]] sf::FloatRect CurrentMarqueeRect() const;
	[[nodiscard]] std::vector<std::shared_ptr<SceneNode>> BuildLiveMarqueeSelection(
	    const std::vector<std::shared_ptr<SceneNode>>& marqueeNodes, bool isCtrlPressed) const;

	SelectCallback _onSelect;
	bool _mousePressed = false;
	bool _isMarqueeSelecting = false;
	sf::Vector2i _dragStartPixel{};
	sf::Vector2f _dragStartWorld{};
	sf::Vector2f _dragCurrentWorld{};
	MarqueeMode _marqueeMode = MarqueeMode::kIntersects;
	std::vector<std::shared_ptr<SceneNode>> _marqueeBaseSelection;
	mutable std::unordered_set<const SceneNode*> _marqueeBaseSelectionPtrSet{};
	std::optional<sf::Vector2i> _lastLiveMarqueeEmitPixel{};
	std::optional<bool> _lastLiveMarqueeEmitCtrl{};
	std::optional<MarqueeMode> _lastLiveMarqueeEmitMode{};
};
