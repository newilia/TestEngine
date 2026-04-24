#include "PongBall.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/App/Utils.h"

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
		auto rigidBody = owner->GetNode()->RequireBehaviour<RigidBodyBehaviour>();
		if (auto speedExcess = utils::Length(rigidBody->_velocity) / owner->GetMaxSpeed(); speedExcess > 1.f) {
			float dampingMultiplier = 1 - speedExcess * dt.asSeconds() * owner->GetSpeedDampingFactor();
			rigidBody->_velocity *= dampingMultiplier;
		}
	}
};

} // namespace

PongBall::PongBall(std::shared_ptr<SceneNode> node) : _node(std::move(node)) {}

void PongBall::SetupBehaviours() {
	_node->AddBehaviour(std::make_shared<PongBallDampingBehaviour>(weak_from_this()));
}

sf::CircleShape* PongBall::GetShape() const {
	auto* c = _node->FindShapeCollider();
	if (!c) {
		return nullptr;
	}
	return dynamic_cast<sf::CircleShape*>(c->GetBaseShape());
}
