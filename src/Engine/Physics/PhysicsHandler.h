#pragma once
#include "AbstractBody.h"
#include "Engine/Updateable.h"
#include "Engine/common.h"
#include "IntersectionDetails.h"
#include "ShapeBody.h"

struct IntersectionDetails;

class PhysicsHandler : public Updateable
{
public:
	virtual ~PhysicsHandler() = default;
	void Update(const sf::Time& dt) override;
	void RegisterBody(shared_ptr<AbstractBody> body);
	void UnregisterBody(AbstractBody* body);

	const auto& GetAllBodies() { return _bodies; }

	void SetSubstepCount(int count) { _subStepsCount = count; }

	void SetGravity(const sf::Vector2f v) { _gravity = v; }

	sf::Vector2f GetGravity() const { return _gravity; }

	void SetGravityEnabled(bool enabled) { _isGravityEnabled = enabled; }

	bool IsGravityEnabled() const { return _isGravityEnabled; }

private:
	static bool CheckBboxIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectPolygonPolygonIntersection(const AbstractBody* body1,
	                                                                           const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectCirclePolygonIntersection(const CircleBody* circle,
	                                                                          const AbstractBody* body);
	static std::optional<IntersectionDetails> DetectCircleCircleIntersection(const CircleBody* circle1,
	                                                                         const CircleBody* circle2);
	static std::optional<SegmentIntersectionPoints> FindSegmentsIntersectionPoint(const Segment& e, const Segment& f);
	static std::optional<SegmentIntersectionPoints>
	FindSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius);
	static void ResolveCollision(const IntersectionDetails& collision);

	void UpdateSubStep(const sf::Time& dt);

	std::list<std::weak_ptr<AbstractBody>> _bodies;
	int _subStepsCount = 1;
	bool _isGravityEnabled = false;
	sf::Vector2f _gravity = {0, 400};
};
