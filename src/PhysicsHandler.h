#pragma once
#include <list>
#include "iUpdateable.h"
#include "PhysicalObject.h"
#include "Singletone.h"

class PhysicsHandler : public iUpdateable, public Singletone<PhysicsHandler> {
public:
	virtual ~PhysicsHandler() = default;
	void update(const sf::Time& dt) override;
	void addObject(const shared_ptr<PhysicalObject>& object) { mObjects.push_back(object); }

private:
	bool checkBboxIntersection(shared_ptr<PhysicalObject>&& obj1, shared_ptr<PhysicalObject>&& obj2);
	void resolveCollision(shared_ptr<PhysicalObject>&& obj1, shared_ptr<PhysicalObject>&& obj2);
	void updateSubStep(const sf::Time& dt);
	void removeExpiredObjects();

	std::list<weak_ptr<PhysicalObject>> mObjects;
	const int mSubStepsCount = 4;
};