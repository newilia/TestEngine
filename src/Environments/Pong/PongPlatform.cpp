#include "PongPlatform.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"

#include <cassert>

namespace {
	class PongPlatformTickBehaviour : public Behaviour
	{
		std::weak_ptr<PongPlatform> _platform;

	public:
		explicit PongPlatformTickBehaviour(std::weak_ptr<PongPlatform> platform) : _platform(std::move(platform)) {}

		void OnUpdate(const sf::Time& dt) override {
			if (auto p = _platform.lock()) {
				p->Update(dt);
			}
		}
	};
} // namespace

PongPlatform::PongPlatform(std::shared_ptr<SceneNode> node) : _node(std::move(node)) {}

void PongPlatform::RegisterTickBehaviour() {
	_node->AddBehaviour(std::make_shared<PongPlatformTickBehaviour>(weak_from_this()));
}

void PongPlatform::Init() {
	if (_controller) {
		_controller->Init();
	}
}

sf::ConvexShape* PongPlatform::GetShape() const {
	auto* c = _node->FindShapeCollider();
	if (!c) {
		return nullptr;
	}
	return dynamic_cast<sf::ConvexShape*>(c->GetBaseShape());
}

sf::FloatRect PongPlatform::GetBbox() const {
	auto* c = _node->FindShapeCollider();
	return c ? c->GetBbox() : sf::FloatRect{};
}

void PongPlatform::SetShapeDimensions(sf::Vector2f size, float curvature, float rotationDeg) {
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
	if (_controller) {
		_controller->Update(dt);
	}
}
