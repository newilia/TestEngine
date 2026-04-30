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

	void SetArrowVisual(std::shared_ptr<VectorArrowVisual> arrow);

	void SetPullForceScale(float v) { _pullForceScale = v; }

	float GetPullForceScale() const { return _pullForceScale; }

	bool IsDebugArrowEnabled() const { return _debugArrowEnabled; }

	void SetDebugArrowEnabled(bool v) { _debugArrowEnabled = v; }

private:
	std::shared_ptr<SceneNode> OnTap(sf::Vector2f mousePos);
	void StopPull();
	void SetPullDestination(const sf::Vector2f& dest);

	SelectTool::SelectCallback _onSelect;
	float _pullForceScale = 1.f;
	sf::Vector2f _destination;
	bool _debugArrowEnabled = true;
	std::weak_ptr<VectorArrowVisual> _arrowVisual;
	std::weak_ptr<SceneNode> _pullingBody;
	bool _isDragging = false;
};

struct PullVisualSetup
{
	std::shared_ptr<SceneNode> root;
	std::shared_ptr<VectorArrowVisual> arrowVisual;
};

/// Root `body_pull` + child arrow visual (no behaviour — pull lives in `PullTool`).
PullVisualSetup CreatePullVisualOverlay();
