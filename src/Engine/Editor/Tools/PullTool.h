#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Tools/IEditorTool.h"
#include "Engine/Editor/Tools/SelectTool.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

class PullTool final : public IEditorTool
{
public:
	explicit PullTool(SelectTool::SelectCallback onSelect);

	bool processEvent(const sf::Event& event) override;
	void onPresent(const sf::Time& dt) override;
	void drawOverlay(sf::RenderWindow& window) override;

	void SetPullForceScale(float v);
	float GetPullForceScale() const;
	bool IsDebugArrowEnabled() const;
	void SetDebugArrowEnabled(bool v);

private:
	std::shared_ptr<SceneNode> OnTap(const sf::Vector2f& screenPixelPos);
	void StopPull();
	void SetPullDestination(const sf::Vector2f& dest);

	SelectTool::SelectCallback _onSelect;
	float _pullForceScale = 1.f;
	sf::Vector2f _destination;
	bool _debugArrowEnabled = true;
	VectorArrow _arrow;
	std::weak_ptr<SceneNode> _pullingBody;
	bool _isDragging = false;
};
