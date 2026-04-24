#pragma once

#include "AbstractBody.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Updateable.h"
#include "Engine/Core/common.h"
#include "IntersectionDetails.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <list>
#include <optional>

class PhysicsHandler : public Updateable
{
public:
	~PhysicsHandler() override = default;
	void Update(const sf::Time& dt) override;
	void RegisterBody(shared_ptr<SceneNode> body);
	void UnregisterBody(SceneNode* body);

	const auto& GetAllBodies() { return _bodies; }

	void SetSubstepCount(int count) { _subStepsCount = count; }

	void SetGravity(const sf::Vector2f v) { _gravity = v; }

	sf::Vector2f GetGravity() const { return _gravity; }

	void SetGravityEnabled(bool enabled) { _isGravityEnabled = enabled; }

	bool IsGravityEnabled() const { return _isGravityEnabled; }

private:
	static bool CheckBboxIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectIntersection(const shared_ptr<SceneNode>& n1,
	                                                             const shared_ptr<SceneNode>& n2);
	static std::optional<IntersectionDetails> DetectPolygonPolygonIntersection(const AbstractBody* body1,
	                                                                           const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectCirclePolygonIntersection(const sf::CircleShape* circle,
	                                                                          const AbstractBody* polygon);
	static std::optional<IntersectionDetails> DetectCircleCircleIntersection(const sf::CircleShape* circle1,
	                                                                         const sf::CircleShape* circle2);
	static std::optional<SegmentIntersectionPoints> FindSegmentsIntersectionPoint(const Segment& e, const Segment& f);
	static std::optional<SegmentIntersectionPoints>
	FindSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius);
	static void ResolveCollision(const IntersectionDetails& collision);

	void UpdateSubStep(const sf::Time& dt);

	std::list<std::weak_ptr<SceneNode>> _bodies;
	int _subStepsCount = 1;
	bool _isGravityEnabled = false;
	sf::Vector2f _gravity = {0, 400};
};
