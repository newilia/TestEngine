#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/UserPullBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <memory>

class SceneNode;
class BodyPullHandler;

struct BodyPullSetup
{
	std::shared_ptr<SceneNode> node;
	std::shared_ptr<BodyPullHandler> handler;
};

class BodyPullHandler : public Behaviour
{
	META_CLASS()
public:
	explicit BodyPullHandler(std::shared_ptr<VectorArrowVisual> arrowVisual);

	void EnableDebugDraw(bool enable) { _isDebugDrawEnabled = enable; }

	void StartPull(sf::Vector2f mousePos);
	void StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode);
	void StopPull();
	void SetPullDestination(sf::Vector2f dest) const;
	void OnUpdate(const sf::Time& dt) override;

private:
	/// @property(name="Debug draw")
	bool _isDebugDrawEnabled = true;
	/// @property(name="Pull force scale", minValue=0.01, maxValue=100.0, step=0.05, dragSpeed=0.05, tooltip="Multiplier
	/// for pull force (FORCE mode); base strength is 100000.")
	float _pullForceScale = 1.f;
	/// @property(name="Default pull mode", minValue=0, maxValue=2, step=1, tooltip="0=Position snap, 1=Force, 2=Set
	/// velocity. Used by StartPull(pos) without explicit mode.")
	int _defaultPullModeIndex = 1;
	std::weak_ptr<SceneNode> _pullingBody;
	std::shared_ptr<VectorArrowVisual> _arrowVisual;
};

/// Родительская нода (body_pull) + дочерняя (body_pull_arrow) с `VectorArrowVisual`.
BodyPullSetup CreateBodyPullOverlay();
