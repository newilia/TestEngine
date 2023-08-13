#pragma once
#include "common.h"

#include "Updateable.h"
#include "AbstractBody.h"
#include "Singletone.h"

class PhysicsHandler : public Updateable, public Singletone<PhysicsHandler> {
public:
	virtual ~PhysicsHandler() = default;
	void update(const sf::Time& dt) override;
	void addBody(const shared_ptr<AbstractBody>& object) { mBodies.push_back(object); }

	static bool checkBboxIntersection(const shared_ptr<AbstractBody>& obj1, const shared_ptr<AbstractBody>& obj2);
	static std::optional<sf::Vector2f> findSegmentsIntersectionPoint(const Segment& E, const Segment& F);
private:
	static void resolveCollision(shared_ptr<AbstractBody>&& body1, shared_ptr<AbstractBody>&& body2, sf::Vector2f collisionPoint);
	void updateSubStep(const sf::Time& dt);
	static std::optional<sf::Vector2f> findCollisionPoint(const shared_ptr<AbstractBody>& obj1, const shared_ptr<AbstractBody>& obj2);

	std::list<weak_ptr<AbstractBody>> mBodies;
	const int mSubStepsCount = 4;
	float mGravity = 0.f;
};