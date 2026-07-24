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

	/// Clones this prefab and parents it under `parent`. Returns null on failure.
	[[nodiscard]] std::shared_ptr<SceneNode> InstantiateOn(const std::shared_ptr<SceneNode>& parent) const;

private:
	std::shared_ptr<SceneNode> _root;
};
