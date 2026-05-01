#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MetaClass.h"
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

class Transform;

class SceneNode : public enable_shared_from_this<SceneNode>,
                  public Updatable,
                  public sf::Drawable,
                  public Engine::IPropertiesProvider
{
	META_CLASS()
public:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void Update(const sf::Time& dt) override; // TODO remove
	void UpdateRec(const sf::Time& dt);       // TODO rename to Update()

	/// After simulation ticks, once per frame before draw (HUD / frame-synced behaviours).
	void NotifyPresentRec(const sf::Time& realFrameDt);

	/// Called once per node when this subtree enters the active scene (`NotifyLifecycleInitRecursive`).
	/// Graph changes (`AddChild`) do not call this; use `NotifyLifecycleInitRecursive` after the subtree is ready.
	virtual void OnInit();

	/// Symmetric to `OnInit` when leaving the active scene or before the subtree is torn down.
	virtual void OnDeinit();

	void RemoveFromParent();
	void SetName(const std::string& name);
	const std::string& GetName() const;
	shared_ptr<SceneNode> GetParent() const;
	const std::vector<shared_ptr<SceneNode>>& GetChildren() const;

	void AddChild(const std::shared_ptr<SceneNode>& child);
	void RemoveChild(SceneNode* child);
	shared_ptr<SceneNode> FindChild(const std::string& id, bool recursively);
	bool HasChild(const std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> FindChildren(const std::string& id, bool recursively);
	void SetVisual(shared_ptr<Visual>&& visual);
	void SetSortingStrategy(const shared_ptr<SortingStrategy>& sorting);

	shared_ptr<Visual> GetVisual() const;
	shared_ptr<Transform> GetTransform() const;
	shared_ptr<SortingStrategy> GetSortingStrategy() const;
	const std::vector<shared_ptr<Behaviour>>& GetBehaviours() const;

	void AddBehaviour(shared_ptr<Behaviour> behaviour);

	template <typename T>
	void RemoveBehaviour() {
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
	shared_ptr<T> FindEntity() const {
		if (auto t = std::dynamic_pointer_cast<T>(GetTransform())) {
			return t;
		}
		if (auto v = std::dynamic_pointer_cast<T>(_visual)) {
			return v;
		}
		if (auto s = std::dynamic_pointer_cast<T>(_sortingStrategy)) {
			return s;
		}
		for (auto& b : _behaviours) {
			if (auto t = std::dynamic_pointer_cast<T>(b)) {
				return t;
			}
		}
		return nullptr;
	}

	template <typename T>
	shared_ptr<T> FindBehaviour() const {
		static_assert(std::is_base_of_v<Behaviour, T>, "FindBehaviour is only for Behaviour types");
		for (auto& b : _behaviours) {
			if (auto t = std::dynamic_pointer_cast<T>(b)) {
				return t;
			}
		}
		return nullptr;
	}

	template <typename T>
	shared_ptr<T> RequireBehaviour() {
		static_assert(std::is_base_of_v<Behaviour, T>, "RequireBehaviour is only for Behaviour types");
		if (auto existing = FindBehaviour<T>()) {
			return existing;
		}
		auto created = std::make_shared<T>();
		AddBehaviour(created);
		return created;
	}

	ShapeColliderBehaviourBase* FindShapeCollider() const; // todo remove

	shared_ptr<SceneNode> FindTopMostNodeAtPoint(const sf::Vector2f& worldPoint, bool tapResponsiveOnly = false);
	void FindNodesAtPoint(const sf::Vector2f& worldPoint, std::vector<shared_ptr<SceneNode>>& result,
	                      bool tapResponsiveOnly = false);
	bool DispatchTapAt(const sf::Vector2f& worldPoint);
	sf::Vector2f GetPosGlobal() const;
	void SetPosGlobal(sf::Vector2f pos);

	void SetEnabled(bool isEnabled);
	void SetVisible(bool isVisible);

	bool IsEnabled() const;
	bool IsVisible() const;

	void NotifyLifecycleInitRecursive();
	void NotifyLifecycleDeinitRecursive();

protected:
	void SetParent(const shared_ptr<SceneNode>& parent);

private:
	void DetachBehaviourForRemove(const shared_ptr<Behaviour>& b);
	bool IsInActiveScene() const;
	shared_ptr<SceneNode> GetSubtreeRoot() const;

private:
	/// @property(name="Name")
	std::string _name;
	/// @property(name="Enabled", setter=SetEnabled)
	bool _isEnabled = true;
	/// @property(name="Visible", setter=SetVisible)
	bool _isVisible = true;

private:
	weak_ptr<SceneNode> _parent;
	std::vector<shared_ptr<SceneNode>> _children;
	mutable shared_ptr<Transform> _transform;
	shared_ptr<Visual> _visual;
	shared_ptr<SortingStrategy> _sortingStrategy;
	std::vector<shared_ptr<Behaviour>> _behaviours;
	bool _wasNodeLifecycleInited = false;
};

using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;
