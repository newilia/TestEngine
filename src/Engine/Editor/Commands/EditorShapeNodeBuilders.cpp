#include "Engine/Editor/Commands/EditorShapeNodeBuilders.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/ConvexShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"

namespace Engine::EditorCommands {

	std::shared_ptr<SceneNode> BuildRectangleShapeNode(const sf::Vector2f centerWorld, const sf::Vector2f size,
	    const bool attachPhysics, const Utils::HsvShapeColors colors) {
		auto node = SceneNode::Create();
		node->SetName("Rectangle");
		auto visual = std::make_shared<RectangleShapeVisual>();
		visual->SetSize(size);
		visual->SetOrigin(size * 0.5f);
		visual->SetFillColor(colors.fill);
		visual->SetOutlineColor(colors.outline);
		visual->SetOutlineThickness(-1.f);
		node->SetVisual(std::move(visual));
		Utils::SetLocalPosToWorld(node, centerWorld);
		if (attachPhysics) {
			node->RequireBehaviour<PhysicsBodyBehaviour>();
		}
		return node;
	}

	std::shared_ptr<SceneNode> BuildCircleShapeNode(const sf::Vector2f centerWorld, const float radius,
	    const bool attachPhysics, const Utils::HsvShapeColors colors) {
		auto node = SceneNode::Create();
		node->SetName("Circle");
		auto visual = std::make_shared<CircleShapeVisual>();
		visual->SetRadius(radius);
		visual->SetFillColor(colors.fill);
		visual->SetOutlineColor(colors.outline);
		visual->SetOutlineThickness(-1.f);
		visual->SetSectorColor(colors.outline);
		node->SetVisual(std::move(visual));
		Utils::SetLocalPosToWorld(node, centerWorld);
		if (attachPhysics) {
			node->RequireBehaviour<PhysicsBodyBehaviour>();
		}
		return node;
	}

	std::shared_ptr<SceneNode> BuildPolygonShapeNode(const sf::Vector2f centerWorld,
	    std::vector<sf::Vector2f> localPoints, const bool attachPhysics, const Utils::HsvShapeColors colors) {
		auto node = SceneNode::Create();
		node->SetName("Polygon");
		auto visual = std::make_shared<ConvexShapeVisual>();
		visual->SetPoints(localPoints);
		visual->SetFillColor(colors.fill);
		visual->SetOutlineColor(colors.outline);
		visual->SetOutlineThickness(-1.f);
		node->SetVisual(std::move(visual));
		Utils::SetLocalPosToWorld(node, centerWorld);
		if (attachPhysics) {
			(void)node->RequireBehaviour<PhysicsBodyBehaviour>();
		}
		return node;
	}

} // namespace Engine::EditorCommands
