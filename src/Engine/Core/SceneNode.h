#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/SceneObjectId.h"
#include "Engine/Core/Transform.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"
#include "Updatable.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;

class Scene;
class PhysicsBodyBehaviour;

class SceneNode final : public enable_shared_from_this<SceneNode>,
                        public Updatable,
                        public sf::Drawable,
                        public Engine::IPropertiesProvider
{
	META_CLASS()

public:
	SceneNode() = default;
	[[nodiscard]] static std::shared_ptr<SceneNode> Create();
	virtual ~SceneNode() = default;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void Update(const sf::Time& dt) override;

	// After `Update`, before draw (frame / HUD).
	void NotifyPresentRec(const sf::Time& wallFrameDt); // TODO is it necessary?

	// Basic properties
	void SetName(const std::string& name);
	const std::string& GetName() const;
	bool IsEnabled() const;
	void SetEnabled(bool isEnabled);
	bool IsVisible() const;
	void SetVisible(bool isVisible);

	Engine::SceneObjectId GetSceneObjectId() const;
	void SetSceneObjectId(Engine::SceneObjectId id);

	// Hierarchy
	shared_ptr<SceneNode> GetParent() const;
	const std::vector<shared_ptr<SceneNode>>& GetChildren() const;
	void AddChild(const std::shared_ptr<SceneNode>& child);
	void AddChildAt(const std::shared_ptr<SceneNode>& child, std::size_t index);
	void RemoveChild(SceneNode* child);
	shared_ptr<SceneNode> FindChild(const std::string& id, bool recursively);
	bool HasChild(const std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> FindChildren(const std::string& id, bool recursively);
	void RemoveFromParent();

	// Transform (embedded local component + cached world)
	Transform& GetLocalTransform();
	const Transform& GetLocalTransform() const;
	const sf::Transform& GetWorldTransform() const;
	void MarkWorldTransformSubtreeDirty() const;

	sf::Vector2f GetLocalPosition() const;
	void SetLocalPosition(sf::Vector2f v);
	sf::Vector2f GetLocalScale() const;
	void SetLocalScale(sf::Vector2f v);
	sf::Angle GetLocalRotation() const;
	void SetLocalRotation(sf::Angle angle);
	sf::Vector2f GetLocalOrigin() const;
	void SetLocalOrigin(sf::Vector2f v);
	void CopyLocalTransformFrom(const Transform& source);

	// Visual & draw order
	shared_ptr<Visual> GetVisual() const;
	void SetVisual(shared_ptr<Visual>&& visual);
	template <typename T>
	shared_ptr<T> RequireVisual();
	template <typename TVisual>
	shared_ptr<TVisual> GetVisual() const;
	shared_ptr<RelativeSortingStrategy> GetSortingStrategy() const;
	void SetSortingStrategy(const shared_ptr<RelativeSortingStrategy>& sorting);

	// Behaviours
	const std::vector<shared_ptr<Behaviour>>& GetBehaviours() const;
	void AddBehaviour(shared_ptr<Behaviour> behaviour);
	void RemoveBehaviour(Behaviour* behaviour);
	template <typename T>
	shared_ptr<T> FindBehaviour() const;
	template <typename T>
	shared_ptr<T> FindBehaviourRec() const;
	template <typename T>
	shared_ptr<T> RequireBehaviour();
	template <typename T>
	void RemoveBehaviour();

	// Hit-testing: subtree nodes under a world point (scene picks top-most / dispatches taps).
	void FindNodesAtPoint(
	    const sf::Vector2f& worldPoint, std::vector<shared_ptr<SceneNode>>& result, bool tapResponsiveOnly = false);

	// Active-scene subtree lifecycle
	void NotifyLifecycleInitRecursive();
	void NotifyLifecycleDeinitRecursive();

	friend class Scene;

private:
	void NotifyEnabledRecursive(bool isEnabled);
	void NotifyVisibleRecursive(bool isVisible);

	void SetParent(const shared_ptr<SceneNode>& parent);
	void DetachBehaviourForRemove(const shared_ptr<Behaviour>& b);
	bool IsInActiveScene() const;
	void PropagateOwningScene(const weak_ptr<Scene>& scene);
	void NotifyActiveSceneObjectIndexDirty() const;
	// prevents UB if _behaviours is modified during iteration loop
	void IterateBehavioursSafely(const std::function<void(shared_ptr<Behaviour>)>& func);

private:
	/// @property
	std::string _name;
	/// @property(setter=SetEnabled)
	bool _isEnabled = true;
	/// @property(setter=SetVisible)
	bool _isVisible = true;

private:
	weak_ptr<SceneNode> _parent;
	std::vector<shared_ptr<SceneNode>> _children;
	Transform _localTransform;
	mutable bool _worldTransformDirty = true;
	mutable sf::Transform _cachedWorldTransform{};
	shared_ptr<Visual> _visual;
	shared_ptr<RelativeSortingStrategy> _sortingStrategy;
	std::vector<shared_ptr<Behaviour>> _behaviours;
	bool _wasNodeLifecycleInited = false;
	Engine::SceneObjectId _sceneObjectId = Engine::kInvalidSceneObjectId;
	weak_ptr<Scene> _owningScene;
};

template <typename TVisual>
shared_ptr<TVisual> SceneNode::GetVisual() const {
	static_assert(std::is_base_of_v<Visual, TVisual>, "GetVisual<T> is only for Visual types");
	if (auto v = std::dynamic_pointer_cast<TVisual>(_visual)) {
		return v;
	}
	return nullptr;
}

template <typename T>
void SceneNode::RemoveBehaviour() {
	for (auto it = _behaviours.begin(); it != _behaviours.end();) {
		if (std::dynamic_pointer_cast<T>(*it)) {
			DetachBehaviourForRemove(*it);
			it = _behaviours.erase(it);
		}
		else {
			++it;
		}
	}
}

template <typename T>
shared_ptr<T> SceneNode::FindBehaviour() const {
	static_assert(std::is_base_of_v<Behaviour, T>, "FindBehaviour is only for Behaviour types");
	for (auto& b : _behaviours) {
		if (auto t = std::dynamic_pointer_cast<T>(b)) {
			return t;
		}
	}
	return nullptr;
}

template <typename T>
shared_ptr<T> SceneNode::FindBehaviourRec() const {
	for (auto& b : _behaviours) {
		if (auto t = std::dynamic_pointer_cast<T>(b)) {
			return t;
		}
	}
	for (auto& child : _children) {
		if (auto found = child->FindBehaviourRec<T>()) {
			return found;
		}
	}
	return nullptr;
}

template <typename T>
shared_ptr<T> SceneNode::RequireBehaviour() {
	static_assert(std::is_base_of_v<Behaviour, T>, "RequireBehaviour is only for Behaviour types");
	if (auto existing = FindBehaviour<T>()) {
		return existing;
	}
	auto created = std::make_shared<T>();
	AddBehaviour(created);
	return created;
}

template <typename T>
shared_ptr<T> SceneNode::RequireVisual() {
	static_assert(std::is_base_of_v<Visual, T>, "RequireVisual is only for Visual types");
	if (auto existing = std::dynamic_pointer_cast<T>(_visual)) {
		return existing;
	}
	auto created = std::make_shared<T>();
	SetVisual(created);
	return created;
}

using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;
