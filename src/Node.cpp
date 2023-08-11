#include "Node.h"

void Node::addChild(std::shared_ptr<Node>&& child) {
	mChildren.push_back(child);
	child->setParent(shared_from_this());
}

shared_ptr<Node> Node::findChild(const std::string& id, bool recursively) {
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

std::vector<shared_ptr<Node>> Node::findChildren(const std::string& id, bool recursively) {
	std::vector<shared_ptr<Node>> result;
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

void Node::update(const sf::Time& dt) {
	update(dt);
	for (auto& child : mChildren) {
		child->update(dt);
	}
}

void Node::removeChild(std::shared_ptr<Node>&& child) {
	auto it = std::find(mChildren.begin(), mChildren.end(), child);
	if (it != mChildren.end()) {
		mChildren.erase(it);
	}
}
