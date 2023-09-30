#pragma once
#include "Engine/common.h"
#include "Engine/Updateable.h"
#include "AbstractBody.h"
#include "IntersectionDetails.h"
#include "ShapeBody.h"

struct IntersectionDetails;

class PhysicsHandler : public Updateable {
public:
	virtual ~PhysicsHandler() = default;
	void update(const sf::Time& dt) override;
	void registerBody(shared_ptr<AbstractBody> body);
	void unregisterBody(AbstractBody* body);
	const auto& getAllBodies() { return mBodies; }
	void setSubstepCount(int count) { mSubStepsCount = count; }
	void setGravity(const sf::Vector2f v) { mGravity = v; }
	sf::Vector2f getGravity() const { return mGravity; }
	void setGravityEnabled(bool enabled) { mIsGravityEnabled = enabled; }
	bool isGravityEnabled() const { return mIsGravityEnabled; }

private:
	static bool checkBboxIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> detectIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> detectPolygonPolygonIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> detectCirclePolygonIntersection(const CircleBody* circle, const AbstractBody* body);
	static std::optional<IntersectionDetails> detectCircleCircleIntersection(const CircleBody* circle1, const CircleBody* circle2);
	static std::optional<SegmentIntersectionPoints> findSegmentsIntersectionPoint(const Segment& E, const Segment& F);
	static std::optional<SegmentIntersectionPoints> findSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius);
	static void resolveCollision(const IntersectionDetails& collision);

	void updateSubStep(const sf::Time& dt);

	std::list<std::weak_ptr<AbstractBody>> mBodies;
	int mSubStepsCount = 1;
	bool mIsGravityEnabled = false;
	sf::Vector2f mGravity = {0, 400};
};
