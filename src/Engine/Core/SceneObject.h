#pragma once

#include <memory>

class SceneNode;

/// Loaded reusable scene subtree asset (SceneObject XML document root).
class SceneObject
{
public:
	explicit SceneObject(std::shared_ptr<SceneNode> root);

	const std::shared_ptr<SceneNode>& GetNode() const;
	operator bool() const;

private:
	std::shared_ptr<SceneNode> _root;
};
