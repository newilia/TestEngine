#include "AbstractBody.h"

#include "EngineInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

void AbstractBody::init() {
	EI()->getPhysicsHandler()->addBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}