#pragma once
#include "AbstractShapeBody.h"
#include "Engine/Utils.h"

template <typename TShape>
class ShapeBody : public AbstractShapeBody
{
	static_assert(std::is_base_of_v<sf::Shape, TShape>);

public:
	ShapeBody() { _shape = new TShape; }

	~ShapeBody() override { delete _shape; }

	TShape* GetShape() const { return dynamic_cast<TShape*>(_shape); }

	void Init() override {
		AbstractBody::Init();
		auto centerOfMass = utils::findCenterOfMass(_shape);
		_shape->setOrigin(centerOfMass);
	}
};

using RectangleBody = ShapeBody<sf::RectangleShape>;
using CircleBody = ShapeBody<sf::CircleShape>;