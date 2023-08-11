#pragma once
#include "iUpdateable.h"

using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::make_shared;

class Node : public iUpdateable, public enable_shared_from_this<Node> {
public:
	const std::string& getId() { return mId; }

	shared_ptr<Node> getParent() const { return mParent.lock(); }

	const auto& getChildren() { return mChildren; }
	void addChild(std::shared_ptr<Node>&& child);
	void removeChild(std::shared_ptr<Node>&& child);
	shared_ptr<Node> findChild(const std::string& id, bool recursively);
	std::vector<shared_ptr<Node>> findChildren(const std::string& id, bool recursively);

	void update(const sf::Time& dt) override;

private:
	void setParent(shared_ptr<Node>&& parent) { mParent = parent; }

	std::string mId;
	weak_ptr<Node> mParent;
	std::vector<shared_ptr<Node>> mChildren;
};
using NodePtr = shared_ptr<Node>;
using NodeWeakPtr = weak_ptr<Node>;