#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <memory>

/// Sets hierarchy selection from scene picks (mouse / primary touch).
class SelectTool final : public IEditorTool
{
public:
	using SelectCallback = std::function<void(std::shared_ptr<SceneNode>)>;

	explicit SelectTool(SelectCallback onSelect);

	[[nodiscard]] bool processEvent(const sf::Event& event) override;
	void drawOverlay(sf::RenderWindow& window) override;
	void drawToolParametersUi() override;

private:
	enum class MarqueeMode
	{
		kIntersects,
		kContains
	};

	[[nodiscard]] sf::FloatRect CurrentMarqueeRect() const;

	SelectCallback _onSelect;
	bool _mousePressed = false;
	bool _isMarqueeSelecting = false;
	sf::Vector2i _dragStartPixel{};
	sf::Vector2f _dragStartWorld{};
	sf::Vector2f _dragCurrentWorld{};
	MarqueeMode _marqueeMode = MarqueeMode::kIntersects;
};
