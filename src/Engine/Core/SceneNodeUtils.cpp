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

	void SortSceneNodesByDrawOrder(std::vector<std::shared_ptr<SceneNode>>& nodes) {
		std::stable_sort(nodes.begin(), nodes.end(),
		    [](const std::shared_ptr<SceneNode>& a, const std::shared_ptr<SceneNode>& b) -> bool {
			    int la = 0;
			    int lb = 0;
			    if (auto sa = a->GetSortingStrategy()) {
				    la = sa->GetPriority();
			    }
			    if (auto sb = b->GetSortingStrategy()) {
				    lb = sb->GetPriority();
			    }
			    return la < lb;
		    });
	}

	sf::Vector2f GetShapePointWorldPos(sf::Shape const* shape, SceneNode const* node, size_t pointIndex) {
		auto tr = node->GetWorldTransform() * shape->getTransform();
		auto p = shape->getPoint(pointIndex);
		return tr.transformPoint(p);
	}
} // namespace Utils
