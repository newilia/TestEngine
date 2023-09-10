#pragma once
#include "AbstractShapeBody.h"
#include "Utils.h"

template <typename TShape>
class ShapeBody : public AbstractShapeBody {
	static_assert(std::is_base_of_v<sf::Shape, TShape>);

public:
	ShapeBody() {
		mShape = new TShape;
	}
	~ShapeBody() override {
		delete mShape;
	}
	TShape* getShape() const {
		return dynamic_cast<TShape*>(mShape);
	}
	void init() override {
		AbstractBody::init();
		auto centerOfMass = utils::findCenterOfMass(mShape);
		mShape->setOrigin(centerOfMass);
	}
};

using RectangleBody = ShapeBody<sf::RectangleShape>;
using CircleBody = ShapeBody<sf::CircleShape>;