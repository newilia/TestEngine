#include "PongPlatform.h"

PongPlatform::PongPlatform()
	: mController(this)
{
}

void PongPlatform::init() {
	ShapeBody::init();
	mController.init();
}

void PongPlatform::initShape(float width, float height, float angleDeg) {
	constexpr int pointCount = 42;
	auto shape = getShape();
	shape->setPointCount(pointCount);
	for (int i = 0; i < pointCount; ++i) {
		float x = static_cast<float>(i * 2) / (pointCount - 1) - 1.f;
		float y = sqrt(1 - x * x);
		sf::Vector2f point(x * width * 0.5f, y * height);
		shape->setPoint(i, point);
	}
	shape->setRotation(angleDeg);
}

void PongPlatform::update(const sf::Time& dt) {
	ShapeBody<sf::ConvexShape>::update(dt);
	mController.update(dt);
}
