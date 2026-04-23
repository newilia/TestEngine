#include "SceneNode.h"

#include "EngineInterface.h"
#include "Physics/PhysicsDebugComponent.h"

#include <cassert>

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
	for (auto& child : _children) {
		child->updateRec(dt);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// states.transform *= getTransform();
	DrawSelf(target, states);
	for (auto& child : _children) {
		child->draw(target, states);
	}

	if (EngineContext::Instance().IsDebugEnabled()) {
		if (auto debugComponent = findComponent<PhysicsDebugComponent>()) {
			debugComponent->draw(target, states);
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
