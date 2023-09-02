#include "AbstractBody.h"

#include "GlobalInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

void AbstractBody::init() {
	GlobalInterface::getInstance()->getPhysicsHandler()->addBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}