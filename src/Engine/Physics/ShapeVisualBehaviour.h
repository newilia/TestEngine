#pragma once

#include "Engine/Behaviour.h"

class AbstractShapeBody;

class ShapeVisualBehaviour : public Behaviour
{
public:
	explicit ShapeVisualBehaviour(AbstractShapeBody* body) : _body(body) {}

	void OnAttached() override;

private:
	AbstractShapeBody* _body = nullptr;
};
