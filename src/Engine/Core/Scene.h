#pragma once

#include "SceneNode.h"
#include "Updatable.h"

#include <SFML/Graphics.hpp>

#include <memory>

/// Owns the scene graph root; hit picking and tap dispatch are scene-level concerns.
class Scene final : public sf::Drawable, public Updatable
{
public:
	Scene();

	[[nodiscard]] std::shared_ptr<SceneNode> GetRoot() const {
		return _root;
	}

	void Update(const sf::Time& dt) override;
	void NotifyPresentRec(const sf::Time& wallFrameDt);
	void NotifyLifecycleInitRecursive();
	void NotifyLifecycleDeinitRecursive();
	void AddChild(const std::shared_ptr<SceneNode>& child);
	void AddChildAt(const std::shared_ptr<SceneNode>& child, std::size_t index);

	bool DispatchTapAt(const sf::Vector2f& worldPoint);
	[[nodiscard]] std::shared_ptr<SceneNode> FindTopMostNodeAtPoint(const sf::Vector2f& worldPoint,
	                                                                bool tapResponsiveOnly = false);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	std::shared_ptr<SceneNode> _root;
};
