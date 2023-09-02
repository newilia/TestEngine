#pragma once
#include "ComponentHolder.h"
#include "Updateable.h"

using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::make_shared;

class SceneNode
	: public enable_shared_from_this<SceneNode>
	, public Updateable
	, public sf::Drawable
	, public sf::Transformable
	, public ComponentHolderBase
{
public:
	void update(const sf::Time& dt) override {}
	void updateRec(const sf::Time& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {}
	virtual void init() {}
	void removeFromParent();

	void setId(const std::string& id) { mId = id; }
	const std::string& getId() { return mId; }
	shared_ptr<SceneNode> getParent() const { return mParent.lock(); }
	const auto& getChildren() { return mChildren; }
	void addChild(std::shared_ptr<SceneNode>&& child);
	void addChild(SceneNode&& child);
	void removeChild(SceneNode* child);
	shared_ptr<SceneNode> findChild(const std::string& id, bool recursively);
	bool hasChild(std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> findChildren(const std::string& id, bool recursively);

private:
	void setParent(shared_ptr<SceneNode>&& parent) { mParent = parent; }

	std::string mId;
	weak_ptr<SceneNode> mParent;
	std::vector<shared_ptr<SceneNode>> mChildren;
};
using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;