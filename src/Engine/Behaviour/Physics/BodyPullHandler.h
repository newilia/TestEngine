#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/UserPullBehaviour.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <memory>

class SceneNode;

class BodyPullHandler : public Behaviour
{
public:
	explicit BodyPullHandler(std::shared_ptr<VectorArrowVisual> arrowVisual);

	void EnableDebugDraw(bool enable) { _isDebugDrawEnabled = enable; }

	void StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode);
	void StopPull();
	void SetPullDestination(sf::Vector2f dest) const;

	void OnUpdate(const sf::Time& dt) override;

private:
	bool _isDebugDrawEnabled = true;
	std::weak_ptr<SceneNode> _pullingBody;
	std::shared_ptr<VectorArrowVisual> _arrowVisual;
};

struct BodyPullSetup
{
	std::shared_ptr<SceneNode> node;
	std::shared_ptr<BodyPullHandler> handler;
};

/// Родительская нода (body_pull) + дочерняя (body_pull_arrow) с `VectorArrowVisual`.
BodyPullSetup CreateBodyPullOverlay();
