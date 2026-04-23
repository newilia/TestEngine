#pragma once
#include "ComponentHolder.h"
#include "Updateable.h"

using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;

class SceneNode : public enable_shared_from_this<SceneNode>,
                  public Updateable,
                  public sf::Drawable,
                  public sf::Transformable,
                  public ComponentHolderBase
{
public:
	void Update(const sf::Time& dt) override {}

	void updateRec(const sf::Time& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	virtual void DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const {}

	virtual void Init() {}

	void removeFromParent();

	void setName(const std::string& name) { _name = name; }

	const std::string& getName() { return _name; }

	shared_ptr<SceneNode> getParent() const { return _parent.lock(); }

	const auto& getChildren() { return _children; }

	void addChild(std::shared_ptr<SceneNode>&& child);
	void addChild(SceneNode&& child);
	void removeChild(SceneNode* child);
	shared_ptr<SceneNode> findChild(const std::string& id, bool recursively);
	bool hasChild(std::shared_ptr<SceneNode>& child);
	std::vector<shared_ptr<SceneNode>> findChildren(const std::string& id, bool recursively);

private:
	void setParent(shared_ptr<SceneNode>&& parent) { _parent = parent; }

	std::string _name;
	weak_ptr<SceneNode> _parent;
	std::vector<shared_ptr<SceneNode>> _children;
};

using NodePtr = shared_ptr<SceneNode>;
using NodeWeakPtr = weak_ptr<SceneNode>;