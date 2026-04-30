#pragma once

#include "Engine/Behaviour/Physics/UserPullBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Tools/IEditorTool.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

/// LMB pull using mode and strength from the Tools panel (replaces `BodyPullHandler` behaviour).
class PullTool final : public IEditorTool
{
public:
	void SetArrowVisual(std::shared_ptr<VectorArrowVisual> arrow);

	void SetPullForceScale(float v) { _pullForceScale = v; }

	[[nodiscard]] float GetPullForceScale() const { return _pullForceScale; }

	/// 0 = Position, 1 = Force, 2 = Velocity
	void SetPullModeIndex(int index) { _pullModeIndex = index; }

	[[nodiscard]] int GetPullModeIndex() const { return _pullModeIndex; }

	void SetDebugArrowEnabled(bool v) { _debugArrowEnabled = v; }

	[[nodiscard]] bool IsDebugArrowEnabled() const { return _debugArrowEnabled; }

	[[nodiscard]] bool processEvent(const sf::Event& event) override;
	void onPresent(const sf::Time& dt) override;

private:
	void StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode);
	void StopPull();
	void SetPullDestination(sf::Vector2f dest) const;
	static UserPullBehaviour::PullMode PullModeFromIndex(int index);

	float _pullForceScale = 1.f;
	int _pullModeIndex = 1;
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
[[nodiscard]] PullVisualSetup CreatePullVisualOverlay();
