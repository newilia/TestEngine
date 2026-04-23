#include "AbstractBody.h"

#include "Engine/EngineInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

AbstractBody::~AbstractBody() {
	EngineContext::instance().getPhysicsHandler()->unregisterBody(this);
}

void AbstractBody::init() {
	EngineContext::instance().getPhysicsHandler()->registerBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}
