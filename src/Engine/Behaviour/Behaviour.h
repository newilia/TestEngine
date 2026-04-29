#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <SFML/System/Time.hpp>

class SceneNode;

/// Optional component on a `SceneNode`. The engine calls lifecycle methods in a fixed order;
/// user code should treat them as engine-only entry points (do not call `OnInit`/`OnDeinit` yourself).
///
/// - **Constructor**: store dependencies and default field values only; avoid `EngineContext` and
///   other globals that may depend on scene activation order.
/// - **OnAttached**: the behaviour is on a node (`GetNode()` works). Use for wiring to other
///   components on the **same** node (e.g. find a collider). Do not register with global subsystems here.
/// - **OnInit**: the node subtree is under the active scene root (`EngineContext::GetScene()`). Use for
///   `PhysicsHandler`, fonts, and other engine-wide setup.
/// - **OnDeinit**: reverse of `OnInit` (unregister from subsystems). Only runs if `OnInit` ran.
/// - **OnDetached**: the behaviour is about to be removed from the node's list; `GetNode()` is still valid
///   for the duration of the call. Runs after `OnDeinit` when both apply.
/// - **OnEnabled** / **OnVisible**: invoked when this node's local `SetEnabled` / `SetVisible` changes the
///   stored flag (not when a parent disables the subtree). Handlers should tolerate repeated calls with the
///   same value if the editor writes the same state.
/// - **OnPresent**: once per displayed frame after all simulation ticks, before scene draw; use for HUD that
///   must follow render rate rather than logic tick count.
class Behaviour : public EntityOnNode
{
	friend class SceneNode;

public:
	~Behaviour() override = default;

	virtual void OnAttached();
	virtual void OnInit();
	virtual void OnDeinit();
	virtual void OnDetached();
	virtual void OnEnabled(bool isEnabled);
	virtual void OnVisible(bool isVisible);
	virtual void OnUpdate(const sf::Time& dt);
	virtual void OnPresent(const sf::Time& realFrameDt);

private:
	bool _wasInited = false;
};
