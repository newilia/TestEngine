#include "SceneNode.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "SceneNode_gen.hpp"

#include <algorithm>
#include <cassert>

namespace {

	void SortChildrenByDrawOrder(std::vector<shared_ptr<SceneNode>>& nodes) {
		std::stable_sort(nodes.begin(), nodes.end(),
		                 [](const shared_ptr<SceneNode>& a, const shared_ptr<SceneNode>& b) {
			                 int la = 0;
			                 int lb = 0;
			                 if (auto sa = a->FindEntity<SortingStrategy>()) {
				                 la = sa->GetSortLayer();
			                 }
			                 if (auto sb = b->FindEntity<SortingStrategy>()) {
				                 lb = sb->GetSortLayer();
			                 }
			                 return la < lb;
		                 });
	}

} // namespace

void SceneNode::SetVisual(shared_ptr<Visual>&& visual) {
	_visual = std::move(visual);
	if (_visual) {
		_visual->AttachTo(shared_from_this());
	}
}

void SceneNode::SetSortingStrategy(shared_ptr<SortingStrategy>&& sorting) {
	_sortingStrategy = std::move(sorting);
	if (_sortingStrategy) {
		_sortingStrategy->AttachTo(shared_from_this());
	}
}

void SceneNode::AddBehaviour(shared_ptr<Behaviour> behaviour) {
	behaviour->AttachTo(shared_from_this());
	_behaviours.push_back(std::move(behaviour));
	_behaviours.back()->OnAttached();
}

void SceneNode::AddChild(std::shared_ptr<SceneNode>&& child) {
	assert(!HasChild(child));

	_children.push_back(child);
	child->SetParent(shared_from_this());
}

void SceneNode::AddChild(SceneNode&& child) {
	child.SetParent(shared_from_this());
	_children.push_back(make_shared<SceneNode>(child));
}

shared_ptr<SceneNode> SceneNode::FindChild(const std::string& id, bool recursively) {
	for (auto& child : _children) {
		if (child->GetName() == id) {
			return child;
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			if (auto grandChild = child->FindChild(id, true)) {
				return grandChild;
			}
		}
	}
	return nullptr;
}

bool SceneNode::HasChild(std::shared_ptr<SceneNode>& child) {
	auto it = std::find(_children.begin(), _children.end(), child);
	return it != _children.end();
}

std::vector<shared_ptr<SceneNode>> SceneNode::FindChildren(const std::string& id, bool recursively) {
	std::vector<shared_ptr<SceneNode>> result;
	for (auto& child : _children) {
		if (child->GetName() == id) {
			result.emplace_back(child);
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			auto grandChildren = child->FindChildren(id, true);
			result.insert(result.end(), grandChildren.begin(), grandChildren.end());
		}
	}
	return result;
}

void SceneNode::UpdateRec(const sf::Time& dt) {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	Update(dt);
	for (auto& b : _behaviours) {
		b->OnUpdate(dt);
	}
	for (auto& child : _children) {
		child->UpdateRec(dt);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	if (_visual) {
		_visual->Draw(target, states);
	}

	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);

	for (auto& child : sorted) {
		child->draw(target, states);
	}

	if (EngineContext::Instance().IsDebugEnabled()) {
		if (auto debugBehaviour = FindBehaviour<PhysicsDebugBehaviour>()) {
			debugBehaviour->DebugDraw(target, states);
		}
	}
}

shared_ptr<SceneNode> SceneNode::FindTopMostTapTarget(sf::Vector2f windowPosition) {
	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
		if (auto hit = (*it)->FindTopMostTapTarget(windowPosition)) {
			return hit;
		}
	}
	if (_visual && _visual->IsTapHandlingEnabled() && _visual->HitTest(windowPosition)) {
		if (!_visual->IsTransparentToTap()) {
			return shared_from_this();
		}
	}
	return nullptr;
}

bool SceneNode::DispatchTapAt(sf::Vector2f windowPosition) {
	if (auto hit = FindTopMostTapTarget(windowPosition)) {
		if (auto v = hit->GetVisual()) {
			v->OnTap(windowPosition);
			return true;
		}
	}
	return false;
}

void SceneNode::RemoveFromParent() {
	if (auto parent = GetParent()) {
		parent->RemoveChild(this);
	}
}

void SceneNode::RemoveChild(SceneNode* child) {
	auto it = std::ranges::find_if(_children.begin(), _children.end(),
	                               [child](const auto& ptr) { return ptr.get() == child; });
	if (it != _children.end()) {
		_children.erase(it);
	}
}
