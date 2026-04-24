#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Visual/Visual.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Sorting/SortingStrategy.h"
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

class SceneNode final : public enable_shared_from_this<SceneNode>,
                        public Updatable,
                        public sf::Drawable,
                        public sf::Transformable
{
public:
	void Update(const sf::Time& dt) override {}

	void UpdateRec(const sf::Time& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const {}

	void Init() {}

	void RemoveFromParent();

	void SetName(const std::string& name) { _name = name; }

	const std::string& GetName() const { return _name; }

	shared_ptr<SceneNode> GetParent() const { return _parent.lock(); }

	const auto& GetChildren() { return _children; }

	void AddChild(std::shared_ptr<SceneNode>&& child);
	void AddChild(SceneNode&& child);
	void RemoveChild(SceneNode* child);
	shared_ptr<SceneNode> FindChild(const std::string& id, bool recursively);
	bool HasChild(std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> FindChildren(const std::string& id, bool recursively);

	void SetVisual(shared_ptr<Visual>&& visual);
	void SetSortingStrategy(shared_ptr<SortingStrategy>&& sorting);

	shared_ptr<Visual> GetVisual() const { return _visual; }

	shared_ptr<SceneNode> FindTopMostTapTarget(sf::Vector2f windowPosition);

	bool DispatchTapAt(sf::Vector2f windowPosition);

	void AddBehaviour(shared_ptr<Behaviour> behaviour);

	ShapeColliderBehaviourBase* FindShapeCollider() const;

	sf::Vector2f GetPosGlobal() const;
	void SetPosGlobal(sf::Vector2f pos);

	template <typename T>
	void RemoveBehaviour() {
		std::erase_if(_behaviours,
		              [](const shared_ptr<Behaviour>& b) { return std::dynamic_pointer_cast<T>(b) != nullptr; });
	}

	/// Searches the node for any attached entity: visual, sorting, or behaviour.
	template <typename T>
	shared_ptr<T> FindEntity() const {
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

	/// Searches only the behaviour list (excludes node visual and sorting attachment).
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

protected:
	void SetParent(shared_ptr<SceneNode>&& parent) { _parent = parent; }

	shared_ptr<Visual> _visual;
	shared_ptr<SortingStrategy> _sortingStrategy;
	std::vector<shared_ptr<Behaviour>> _behaviours;

	std::string _name;
	weak_ptr<SceneNode> _parent;
	std::vector<shared_ptr<SceneNode>> _children;
};

inline ShapeColliderBehaviourBase* SceneNode::FindShapeCollider() const {
	for (auto& b : _behaviours) {
		if (auto s = std::dynamic_pointer_cast<ShapeColliderBehaviourBase>(b)) {
			return s.get();
		}
	}
	return nullptr;
}

inline sf::Vector2f SceneNode::GetPosGlobal() const {
	if (auto* c = FindShapeCollider()) {
		return c->GetPosGlobal();
	}
	return getPosition();
}

inline void SceneNode::SetPosGlobal(sf::Vector2f pos) {
	if (auto* c = FindShapeCollider()) {
		c->SetPosGlobal(pos);
	}
	else {
		setPosition(pos);
	}
}

using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;
