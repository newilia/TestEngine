#pragma once

#include "Behaviour.h"
#include "NodeVisual.h"
#include "SortingStrategyEntity.h"
#include "Updateable.h"

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

class SceneNode : public enable_shared_from_this<SceneNode>,
                  public Updateable,
                  public sf::Drawable,
                  public sf::Transformable
{
public:
	void Update(const sf::Time& dt) override {}

	void updateRec(const sf::Time& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	virtual void DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const {}

	virtual void Init() {}

	void removeFromParent();

	void setName(const std::string& name) { _name = name; }

	const std::string& getName() const { return _name; }

	shared_ptr<SceneNode> getParent() const { return _parent.lock(); }

	const auto& getChildren() { return _children; }

	void addChild(std::shared_ptr<SceneNode>&& child);
	void addChild(SceneNode&& child);
	void removeChild(SceneNode* child);
	shared_ptr<SceneNode> findChild(const std::string& id, bool recursively);
	bool hasChild(std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> findChildren(const std::string& id, bool recursively);

	void SetVisual(shared_ptr<NodeVisual>&& visual);
	void SetSortingStrategy(shared_ptr<SortingStrategyEntity>&& sorting);

	void AddBehaviour(shared_ptr<Behaviour>&& behaviour);

	template <typename T>
	void RemoveBehaviour() {
		std::erase_if(_behaviours,
		              [](const shared_ptr<Behaviour>& b) { return std::dynamic_pointer_cast<T>(b) != nullptr; });
	}

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

	template <typename T>
	shared_ptr<T> RequireEntity() {
		static_assert(std::is_base_of_v<Behaviour, T>, "RequireEntity is only for Behaviour types");
		if (auto existing = FindEntity<T>()) {
			return existing;
		}
		auto created = std::make_shared<T>();
		shared_ptr<Behaviour> asBase = created;
		AddBehaviour(std::move(asBase));
		return created;
	}

protected:
	void setParent(shared_ptr<SceneNode>&& parent) { _parent = parent; }

	shared_ptr<NodeVisual> _visual;
	shared_ptr<SortingStrategyEntity> _sortingStrategy;
	std::vector<shared_ptr<Behaviour>> _behaviours;

	std::string _name;
	weak_ptr<SceneNode> _parent;
	std::vector<shared_ptr<SceneNode>> _children;
};

using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;
