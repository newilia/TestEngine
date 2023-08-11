#pragma once

#include "Node.h"
#include "ComponentHolder.h"
#include "PhysicalComponent.h"

class PhysicalObject : public Node, public ComponentHolder<PhysicalComponent> {
public:
	PhysicalObject();
private:

};