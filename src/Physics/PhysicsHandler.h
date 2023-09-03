#pragma once
#include "common.h"
#include "Updateable.h"
#include "AbstractBody.h"
#include "CollisionDetails.h"

struct CollisionDetails;

class PhysicsHandler : public Updateable {
public:
	virtual ~PhysicsHandler() = default;
	void update(const sf::Time& dt) override;
	void addBody(const shared_ptr<AbstractBody>& object) { mBodies.push_back(object); }
	const auto& getAllBodies() { return mBodies; }
private:
	static bool checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2);
	static std::optional<CollisionDetails> detectCollision(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2);
	static std::optional<SegmentIntersectionPoints> findSegmentsIntersectionPoint(const Segment& E, const Segment& F);
	static void resolveCollision(const CollisionDetails& collision, const sf::Time& dt);

	void updateSubStep(const sf::Time& dt);

	std::list<weak_ptr<AbstractBody>> mBodies;
	const int mSubStepsCount = 1;
	float mGravity = 0.f;
	bool mDebugDrawEnabled = true;
};
