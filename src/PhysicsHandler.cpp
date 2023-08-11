#include "PhysicsHandler.h"
#include "PhysicalObject.h"

void PhysicsHandler::update(const sf::Time& dt) {
	removeExpiredObjects();
	for (int i = 0; i < mSubStepsCount; ++i) {
		updateSubStep(dt / static_cast<float>(mSubStepsCount));
	}
}

bool PhysicsHandler::checkBboxIntersection(shared_ptr<PhysicalObject>&& obj1, shared_ptr<PhysicalObject>&& obj2) {
	return false;
}

void PhysicsHandler::resolveCollision(shared_ptr<PhysicalObject>&& obj1, shared_ptr<PhysicalObject>&& obj2) {

}

void PhysicsHandler::updateSubStep(const sf::Time& dt) {
	for (auto it1 = mObjects.begin(); it1 != mObjects.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mObjects.end(); ++it2) {
			if (!checkBboxIntersection(it1->lock(), it2->lock())) {
				continue;
			}
			resolveCollision(it1->lock(), it2->lock());
		}
	}
}

void PhysicsHandler::removeExpiredObjects() {
	auto [first, last] = std::ranges::remove_if(mObjects, [](auto ptr) {
		return ptr.expired();
	});
	mObjects.erase(first, last);
}
