#include "AbstractBody.h"

#include "Engine/EngineInterface.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() = default;

AbstractBody::~AbstractBody() {
	EngineContext::Instance().GetPhysicsHandler()->UnregisterBody(this);
}

void AbstractBody::Init() {
	RequireEntity<PhysicalBehaviour>();
	EngineContext::Instance().GetPhysicsHandler()->RegisterBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}
