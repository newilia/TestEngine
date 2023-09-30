#include "PongPlatform.h"

#include <cassert>

PongPlatform::PongPlatform()
{
}

void PongPlatform::init() {
	ShapeBody::init();
	if (mController) {
		mController->init();
	}
}

void PongPlatform::setShapeDimensions(sf::Vector2f size, float curvature, float rotationDeg) {
	assert(curvature >= 0.f && curvature <= 1.f);

	constexpr int pointCount = 42;
	auto shape = getShape();
	shape->setPointCount(pointCount);
	for (int i = 0; i < pointCount; ++i) {
		float x = (static_cast<float>(i) / (pointCount - 1) - 0.5f) * 2.0f;
		float y = sqrt(1 - x * x * curvature);
		sf::Vector2f point(x * size.x * 0.5f, y * size.y);
		shape->setPoint(i, point);
	}
	shape->setRotation(rotationDeg);
}

void PongPlatform::update(const sf::Time& dt) {
	ShapeBody::update(dt);
	if (mController) {
		mController->update(dt);
	}
}
