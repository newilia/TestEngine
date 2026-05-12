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
	explicit PullTool(SelectTool::SelectCallback onSelect = nullptr);

	bool ProcessEvent(const sf::Event& event) override;
	void Update(const sf::Time& dt) override;
	void DrawOverlay(sf::RenderWindow& window) override;
	void DrawToolParametersUi() override;

	float GetPullForceScale() const;
	void SetPullForceScale(float v);
	bool IsDebugArrowEnabled() const;
	void SetDebugArrowEnabled(bool v);

private:
	std::shared_ptr<SceneNode> FindBodyAtPoint(const sf::Vector2f& screenPixelPos);
	void StopPull();
	void SetPullDestination(const sf::Vector2f& dest);

private:
	SelectTool::SelectCallback _onSelect;
	float _pullForceScale = 1.f;
	float _dampening = 0.05f;
	sf::Vector2f _destination;
	bool _debugArrowEnabled = true;
	VectorArrowShape _arrow;
	std::weak_ptr<SceneNode> _pullingBody;
	bool _isDragging = false;
};
