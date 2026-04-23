#include "PongPlatform.h"

#include <cassert>

PongPlatform::PongPlatform() {}

void PongPlatform::Init() {
	ShapeBody::Init();
	if (_controller) {
		_controller->Init();
	}
}

void PongPlatform::setShapeDimensions(sf::Vector2f size, float curvature, float rotationDeg) {
	assert(curvature >= 0.f && curvature <= 1.f);

	constexpr int pointCount = 42;
	auto shape = GetShape();
	shape->setPointCount(pointCount);
	for (int i = 0; i < pointCount; ++i) {
		float x = (static_cast<float>(i) / (pointCount - 1) - 0.5f) * 2.0f;
		float y = sqrt(1 - x * x * curvature);
		sf::Vector2f point(x * size.x * 0.5f, y * size.y);
		shape->setPoint(i, point);
	}
	shape->setRotation(sf::degrees(rotationDeg));
}

void PongPlatform::Update(const sf::Time& dt) {
	ShapeBody::Update(dt);
	if (_controller) {
		_controller->Update(dt);
	}
}
