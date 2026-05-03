#include "PongBall.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/CircleShape.hpp>

namespace {
	class PongBallDampingBehaviour : public Behaviour
	{
		std::weak_ptr<PongBall> _owner;

	public:
		explicit PongBallDampingBehaviour(std::weak_ptr<PongBall> owner) : _owner(std::move(owner)) {}

		void OnUpdate(const sf::Time& dt) override {
			auto owner = _owner.lock();
			if (!owner) {
				return;
			}
			auto rigidBody = owner->GetNode()->RequireBehaviour<PhysicsBodyBehaviour>();
			if (auto speedExcess = Utils::Length(rigidBody->GetVelocity()) / owner->GetMaxSpeed(); speedExcess > 1.f) {
				float dampingMultiplier = 1 - speedExcess * dt.asSeconds() * owner->GetSpeedDampingFactor();
				rigidBody->ScaleVelocity(dampingMultiplier);
			}
		}
	};
} // namespace

PongBall::PongBall(std::shared_ptr<SceneNode> node) : _node(std::move(node)) {}

std::shared_ptr<SceneNode> PongBall::GetNode() const {
	return _node;
}

float PongBall::GetMaxSpeed() const {
	return _targetSpeed;
}

void PongBall::SetMaxSpeed(float maxSpeed) {
	_targetSpeed = maxSpeed;
}

float PongBall::GetSpeedDampingFactor() const {
	return _speedDampingFactor;
}

void PongBall::SetSpeedDampingFactor(float speedDampingFactor) {
	_speedDampingFactor = speedDampingFactor;
}

void PongBall::SetupBehaviours() {
	_node->AddBehaviour(std::make_shared<PongBallDampingBehaviour>(weak_from_this()));
}

sf::CircleShape* PongBall::GetShape() const {
	auto* c = _node->FindPhysicsBody();
	if (!c) {
		return nullptr;
	}
	return dynamic_cast<sf::CircleShape*>(c->GetShape());
}
