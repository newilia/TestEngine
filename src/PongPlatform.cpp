#include "PongPlatform.h"

PongPlatform::PongPlatform()
{
}

void PongPlatform::init() {
	ShapeBody::init();
	if (mController) {
		mController->init();
	}
}

void PongPlatform::setShapeDimensions(float width, float height, float rotationDeg) {
	constexpr int pointCount = 42;
	auto shape = getShape();
	shape->setPointCount(pointCount);
	for (int i = 0; i < pointCount; ++i) {
		float x = static_cast<float>(i * 2) / (pointCount - 1) - 1.f;
		float y = sqrt(1 - x * x);
		sf::Vector2f point(x * width * 0.5f, y * height);
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
