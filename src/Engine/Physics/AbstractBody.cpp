#include "AbstractBody.h"

#include "Engine/EngineInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

AbstractBody::~AbstractBody() {
	EngineContext::Instance().GetPhysicsHandler()->UnregisterBody(this);
}

void AbstractBody::Init() {
	EngineContext::Instance().GetPhysicsHandler()->RegisterBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}
