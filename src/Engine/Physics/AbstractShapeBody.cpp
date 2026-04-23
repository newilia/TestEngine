#include "AbstractShapeBody.h"

#include "ShapeVisualBehaviour.h"

void AbstractShapeBody::Init() {
	AbstractBody::Init();
	if (!FindEntity<ShapeVisualBehaviour>()) {
		AddBehaviour(std::make_shared<ShapeVisualBehaviour>(this));
	}
}
