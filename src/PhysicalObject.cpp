#include "PhysicalObject.h"
#include "PhysicsHandler.h"

PhysicalObject::PhysicalObject() {
	PhysicsHandler::getInstance()->addObject(dynamic_pointer_cast<PhysicalObject>(shared_from_this()));
}
