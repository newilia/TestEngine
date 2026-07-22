#include "Engine/Core/SceneNodeUtils.h"

#include "Engine/Behaviour/PointLightBehaviour.h"
#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <algorithm>

namespace Utils {
	sf::Vector2f GetWorldPos(const std::shared_ptr<const SceneNode>& node) {
		if (node) {
			return node->GetWorldTransform().transformPoint(sf::Vector2f{});
		}
		return {};
	}

	std::optional<sf::FloatRect> TryGetNodeVisualWorldBounds(const std::shared_ptr<const SceneNode>& node) {
		if (!node) {
			return std::nullopt;
		}
		const auto visual = node->GetVisual();
		if (!visual) {
			return std::nullopt;
		}
		sf::Transform fullTransform = node->GetWorldTransform();
		if (const auto* visualTransform = visual->GetTransform()) {
			fullTransform *= *visualTransform;
		}
		return fullTransform.transformRect(visual->GetLocalBounds());
	}

	sf::Vector2f GetNodeCameraFocusWorldPoint(const std::shared_ptr<const SceneNode>& node) {
		if (const auto bounds = TryGetNodeVisualWorldBounds(node)) {
			return bounds->position + bounds->size * 0.5f;
		}
		return GetWorldPos(node);
	}

	void SetLocalPosToWorld(const std::shared_ptr<SceneNode>& node, sf::Vector2f worldPos) {
		const auto n = Verify(node);
		if (auto parent = n->GetParent()) {
			const sf::Vector2f local = parent->GetWorldTransform().getInverse().transformPoint(worldPos);
			assert(!IsNan(local));
			n->SetLocalPosition(local);
		}
		else {
			n->SetLocalPosition(worldPos);
		}
	}

	void AddLightSource(SceneNode* node, float intensity, float radius, sf::Color color) {
		auto pl = node->RequireBehaviour<PointLightBehaviour>();
		pl->SetLightColor(color);
		pl->SetIntensity(intensity);
		pl->SetRadius(radius);
	}

	void AddLightReceiver(SceneNode* node, float diffusion, bool isBevelEmboss, float bevelWidth) {
		auto recv = node->RequireBehaviour<ShapeLightReceiverBehaviour>();
		recv->SetBevelEmbossMode(isBevelEmboss);
		recv->SetBevelWidth(bevelWidth);
		recv->SetDiffusion(diffusion);
		recv->SetEaseInCirc(true);
	}

	namespace {
		void CollectSceneNodesForDrawRec(
		    const std::shared_ptr<SceneNode>& node, int parentSortKey, std::vector<SceneNodeDrawEntry>& out) {
			if (!node || !node->IsEnabled() || !node->IsVisible()) {
				return;
			}

			int sortKey = 0;
			if (const auto sorting = node->GetSortingStrategy()) {
				if (sorting->GetType() == SortingStrategyType::Absolute) {
					sortKey = sorting->GetSortKey();
				}
				else {
					sortKey += sorting->GetSortKey();
				}
			}
			else {
				sortKey = parentSortKey + 1;
			}

			out.push_back({node, sortKey, out.size()});

			for (const auto& child : node->GetChildren()) {
				CollectSceneNodesForDrawRec(child, sortKey, out);
			}
		}
	} // namespace

	void CollectSceneNodesForDraw(const std::shared_ptr<SceneNode>& root, std::vector<SceneNodeDrawEntry>& out) {
		out.clear();
		if (!root) {
			return;
		}
		CollectSceneNodesForDrawRec(root, -1, out);
	}

	void StableSortDrawEntriesAscending(std::vector<SceneNodeDrawEntry>& entries) {
		std::stable_sort(
		    entries.begin(), entries.end(), [](const SceneNodeDrawEntry& a, const SceneNodeDrawEntry& b) -> bool {
			    return a.sortKey < b.sortKey;
		    });
	}

	sf::Vector2f GetShapePointWorldPos(sf::Shape const* shape, SceneNode const* node, size_t pointIndex) {
		auto tr = node->GetWorldTransform() * shape->getTransform();
		auto p = shape->getPoint(pointIndex);
		return tr.transformPoint(p);
	}
} // namespace Utils
