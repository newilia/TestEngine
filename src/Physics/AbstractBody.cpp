#include "AbstractBody.h"

#include "EngineInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

AbstractBody::~AbstractBody() {
	EI()->getPhysicsHandler()->unregisterBody(this);
}

void AbstractBody::init() {
	EI()->getPhysicsHandler()->registerBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}
