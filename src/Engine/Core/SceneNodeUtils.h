#pragma once

#include "SFML/Graphics.hpp"

#include <memory>
#include <optional>
#include <vector>

class SceneNode;

namespace Utils {
	template <typename T, typename U>
	std::shared_ptr<T> SharedPtrCast(const U* ptr) {
		return std::dynamic_pointer_cast<T>((const_cast<U*>(ptr))->shared_from_this());
	}

	sf::Vector2f GetWorldPos(const std::shared_ptr<const SceneNode>& node);

	/// World-space AABB of the node's `Visual::GetLocalBounds()` after full transform, if a visual exists.
	std::optional<sf::FloatRect> TryGetNodeVisualWorldBounds(const std::shared_ptr<const SceneNode>& node);

	/// Prefer center of `TryGetNodeVisualWorldBounds`; otherwise `GetWorldPos` (node origin in world space).
	sf::Vector2f GetNodeCameraFocusWorldPoint(const std::shared_ptr<const SceneNode>& node);

	void SetLocalPosToWorld(const std::shared_ptr<SceneNode>& node, sf::Vector2f pos);

	void AddLightSource(SceneNode* node, float intensity, float radius, sf::Color color);
	void AddLightReceiver(SceneNode* node, float diffusion, bool isBevelEmboss, float bevelWidth = 0.f);

	void SortSceneNodesByDrawOrder(std::vector<std::shared_ptr<SceneNode>>& nodes);

	sf::Vector2f GetShapePointWorldPos(sf::Shape const* shape, SceneNode const* node, size_t pointIndex);
} // namespace Utils
