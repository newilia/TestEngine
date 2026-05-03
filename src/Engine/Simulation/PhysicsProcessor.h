#pragma once

#include "AttractionField.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Updatable.h"
#include "Engine/Core/common.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <list>
#include <memory>
#include <optional>

class PhysicsProcessor : public Updatable
{
public:
	~PhysicsProcessor() override = default;
	void Update(const sf::Time& dt) override;
	void RegisterBody(shared_ptr<SceneNode> body);
	void UnregisterBody(SceneNode* body);
	const std::list<std::weak_ptr<SceneNode>>& GetAllBodies() const;
	void SetGravity(const sf::Vector2f v);
	sf::Vector2f GetGravity() const;
	void SetGravityEnabled(bool enabled);
	bool IsGravityEnabled() const;
	std::shared_ptr<AttractionField> GetAttractionField() const;

private:
	static bool CheckBboxIntersection(const PhysicsBodyBehaviour* body1, const PhysicsBodyBehaviour* body2);
	static std::optional<IntersectionDetails> DetectIntersection(const shared_ptr<SceneNode>& n1,
	                                                             const shared_ptr<SceneNode>& n2,
	                                                             PhysicsBodyBehaviour* c1, PhysicsBodyBehaviour* c2,
	                                                             bool bboxAlreadyVerified);
	static std::optional<IntersectionDetails> DetectPolygonPolygonIntersection(const PhysicsBodyBehaviour* body1,
	                                                                           const PhysicsBodyBehaviour* body2);
	static std::optional<IntersectionDetails> DetectCirclePolygonIntersection(const SceneNode& circleNode,
	                                                                          const sf::CircleShape* circle,
	                                                                          const PhysicsBodyBehaviour* polygon);
	static std::optional<IntersectionDetails> DetectCircleCircleIntersection(const SceneNode& node1,
	                                                                         const sf::CircleShape* circle1,
	                                                                         const SceneNode& node2,
	                                                                         const sf::CircleShape* circle2);
	static std::optional<SegmentIntersectionPoints> FindSegmentsIntersectionPoint(const Segment& e, const Segment& f);
	static std::optional<SegmentIntersectionPoints>
	FindSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius);
	static void ResolveCollision(const IntersectionDetails& collision);
	std::list<std::weak_ptr<SceneNode>> _bodies;
	sf::Vector2f _gravity = {0, 400};
	bool _isGravityEnabled = false;
	std::shared_ptr<AttractionField> _inverseSquareField = std::make_shared<AttractionField>();
};
