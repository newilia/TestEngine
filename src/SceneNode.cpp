#include "SceneNode.h"

#include <cassert>

#include "DebugComponent.h"

void SceneNode::addChild(std::shared_ptr<SceneNode>&& child) {
	assert(!hasChild(child));

	mChildren.push_back(child);
	child->setParent(shared_from_this());
}

void SceneNode::addChild(SceneNode&& child) {
	child.setParent(shared_from_this());
	mChildren.push_back(make_shared<SceneNode>(child));
}

shared_ptr<SceneNode> SceneNode::findChild(const std::string& id, bool recursively) {
	for (auto & child : mChildren) {
		if (child->getId() == id) {
			return child;
		}
	}
	if (recursively) {
		for (auto& child : mChildren) {
			if (auto grandChild = child->findChild(id, true)) {
				return grandChild;
			}
		}
	}
	return nullptr;
}

bool SceneNode::hasChild(std::shared_ptr<SceneNode>& child) {
	auto it = std::find(mChildren.begin(), mChildren.end(), child);
	return it != mChildren.end();
}

std::vector<shared_ptr<SceneNode>> SceneNode::findChildren(const std::string& id, bool recursively) {
	std::vector<shared_ptr<SceneNode>> result;
	for (auto& child : mChildren) {
		if (child->getId() == id) {
			result.emplace_back(child);
		}
	}
	if (recursively) {
		for (auto& child : mChildren) {
			auto grandChildren = child->findChildren(id, true);
			result.insert(result.end(), grandChildren.begin(), grandChildren.end());
		}
	}
	return result;
} 

void SceneNode::updateRec(const sf::Time& dt) {
	update(dt);
	for (auto& child : mChildren) {
		child->updateRec(dt);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	//states.transform *= getTransform();
	drawSelf(target, states);
	for (auto& child : mChildren) {
		child->draw(target, states);
	}
	if (auto debugComponent = findComponent<DebugComponent>()) {
		debugComponent->draw(target, states);
	}
}

void SceneNode::removeFromParent() {
	if (auto parent = getParent()) {
		parent->removeChild(this);
	}
}

void SceneNode::removeChild(SceneNode* child) {
	auto it = std::find_if(mChildren.begin(), mChildren.end(), [child](const auto& ptr) {
		return ptr.get() == child;
	});
	if (it != mChildren.end()) {
		mChildren.erase(it);
	}
}
