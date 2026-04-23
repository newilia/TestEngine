#include "SceneNode.h"

#include <algorithm>

#include "EngineInterface.h"
#include "Physics/PhysicsDebugBehaviour.h"

#include <cassert>

void SceneNode::SetVisual(shared_ptr<NodeVisual>&& visual) {
	_visual = std::move(visual);
	if (_visual) {
		_visual->AttachTo(shared_from_this());
	}
}

void SceneNode::SetSortingStrategy(shared_ptr<SortingStrategyEntity>&& sorting) {
	_sortingStrategy = std::move(sorting);
	if (_sortingStrategy) {
		_sortingStrategy->AttachTo(shared_from_this());
	}
}

void SceneNode::AddBehaviour(shared_ptr<Behaviour>&& behaviour) {
	behaviour->AttachTo(shared_from_this());
	_behaviours.push_back(std::move(behaviour));
	_behaviours.back()->OnAttached();
}

void SceneNode::addChild(std::shared_ptr<SceneNode>&& child) {
	assert(!hasChild(child));

	_children.push_back(child);
	child->setParent(shared_from_this());
}

void SceneNode::addChild(SceneNode&& child) {
	child.setParent(shared_from_this());
	_children.push_back(make_shared<SceneNode>(child));
}

shared_ptr<SceneNode> SceneNode::findChild(const std::string& id, bool recursively) {
	for (auto& child : _children) {
		if (child->getName() == id) {
			return child;
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			if (auto grandChild = child->findChild(id, true)) {
				return grandChild;
			}
		}
	}
	return nullptr;
}

bool SceneNode::hasChild(std::shared_ptr<SceneNode>& child) {
	auto it = std::find(_children.begin(), _children.end(), child);
	return it != _children.end();
}

std::vector<shared_ptr<SceneNode>> SceneNode::findChildren(const std::string& id, bool recursively) {
	std::vector<shared_ptr<SceneNode>> result;
	for (auto& child : _children) {
		if (child->getName() == id) {
			result.emplace_back(child);
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			auto grandChildren = child->findChildren(id, true);
			result.insert(result.end(), grandChildren.begin(), grandChildren.end());
		}
	}
	return result;
}

void SceneNode::updateRec(const sf::Time& dt) {
	Update(dt);
	for (auto& b : _behaviours) {
		b->OnUpdate(dt);
	}
	for (auto& child : _children) {
		child->updateRec(dt);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// states.transform *= getTransform();
	DrawSelf(target, states);
	if (_visual) {
		_visual->Draw(target, states);
	}

	std::vector<shared_ptr<SceneNode>> sorted = _children;
	std::stable_sort(sorted.begin(), sorted.end(), [](const shared_ptr<SceneNode>& a, const shared_ptr<SceneNode>& b) {
		int la = 0;
		int lb = 0;
		if (auto sa = a->FindEntity<SortingStrategyEntity>()) {
			la = sa->GetSortLayer();
		}
		if (auto sb = b->FindEntity<SortingStrategyEntity>()) {
			lb = sb->GetSortLayer();
		}
		return la < lb;
	});

	for (auto& child : sorted) {
		child->draw(target, states);
	}

	if (EngineContext::Instance().IsDebugEnabled()) {
		if (auto debugBehaviour = FindEntity<PhysicsDebugBehaviour>()) {
			debugBehaviour->DrawDebug(target, states);
		}
	}
}

void SceneNode::removeFromParent() {
	if (auto parent = getParent()) {
		parent->removeChild(this);
	}
}

void SceneNode::removeChild(SceneNode* child) {
	auto it = std::ranges::find_if(_children.begin(), _children.end(),
	                              [child](const auto& ptr) { return ptr.get() == child; });
	if (it != _children.end()) {
		_children.erase(it);
	}
}
