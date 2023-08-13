#include "AbstractBody.h"
#include "PhysicsHandler.h"

AbstractBody::AbstractBody() {
	requireComponent<PhysicalComponent>();
}

void AbstractBody::init() {
	PhysicsHandler::getInstance()->addBody(dynamic_pointer_cast<AbstractBody>(shared_from_this()));
}
