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
	void setSubstepCount(int count) { mSubStepsCount = count; }
	void setGravity(const sf::Vector2f v) { mGravity = v; }
	sf::Vector2f getGravity() const { return mGravity; }
	void setGravityEnabled(bool enabled) { mIsGravityEnabled = enabled; }
	bool isGravityEnabled() const { return mIsGravityEnabled; }

private:
	static bool checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2);
	static std::optional<CollisionDetails> detectCollision(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2);
	static std::optional<SegmentIntersectionPoints> findSegmentsIntersectionPoint(const Segment& E, const Segment& F);
	static void resolveCollision(const CollisionDetails& collision);

	void updateSubStep(const sf::Time& dt);

	std::list<weak_ptr<AbstractBody>> mBodies;
	int mSubStepsCount = 1;
	bool mIsGravityEnabled = false;
	sf::Vector2f mGravity = {0, 400};
	bool mDebugDrawEnabled = true;
};
