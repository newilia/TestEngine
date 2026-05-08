#include "GameControllerBehaviour.h"

#include "Engine/Behaviour/RadialUvWarpBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "GameControllerBehaviour.generated.hpp"

#include <cmath>

namespace BallGame1 {
	void GameControllerBehaviour::OnInit() {
		EventHandlerBehaviourBase::OnInit();
	}

	void GameControllerBehaviour::OnUpdate(const sf::Time& dt) {
		EventHandlerBehaviourBase::OnUpdate(dt);
	}

	void GameControllerBehaviour::OnEvent(const sf::Event& event) {
		if (auto e = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (e->button == sf::Mouse::Button::Left) {
				Shoot();
			}
		}
	}

	void GameControllerBehaviour::SetFieldNode(const std::weak_ptr<SceneNode>& fieldNode) {
		_fieldNode = fieldNode;
	}

	void GameControllerBehaviour::SetGunNode(const std::weak_ptr<SceneNode>& gunNode) {
		_gunNode = gunNode;
	}

	void GameControllerBehaviour::SetScoreNode(const std::weak_ptr<SceneNode>& scoreNode) {
		_scoreNode = scoreNode;
	}

	void GameControllerBehaviour::SetBallParameters(
	    float mass, float restitution, float radius, const sf::Color& color) {
		_ballMass = mass;
		_ballRestitution = restitution;
		_ballRadius = radius;
		_ballColor = color;
	}

	void GameControllerBehaviour::StartNewGame() {
		auto ballNode = CreateBallNode();
		AttachBallToGun(ballNode);
	}

	void GameControllerBehaviour::Shoot() {
		auto ballNode = _ballNode.lock();
		auto gunNode = _gunNode.lock();

		if (!ballNode || !gunNode) {
			return;
		}
		auto gunRotation = gunNode->GetLocalTransform()->GetRotation().asRadians();
		auto ballBody = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
		auto moveDirection = sf::Vector2f(std::sin(gunRotation), -std::cos(gunRotation));
		ballBody->SetVelocity(moveDirection * _ballSpeed);
		DetachBallFromGun();
		AttachBallToGun(CreateBallNode());
	}

	std::shared_ptr<SceneNode> GameControllerBehaviour::CreateBallNode() const {
		auto ballNode = make_shared<SceneNode>();
		ballNode->SetName("Ball");

		auto body = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
		body->SetMass(_ballMass);
		body->SetRestitution(_ballRestitution);

		auto visual = ballNode->RequireVisual<CircleShapeVisual>();
		visual->SetRadius(_ballRadius);
		visual->SetFillColor(_ballColor);

		ballNode->RequireBehaviour<RadialUvWarpBehaviour>();
		return ballNode;
	}

	void GameControllerBehaviour::AttachBallToGun(const std::shared_ptr<SceneNode>& ballNode) {
		auto gunNode = _gunNode.lock();
		if (!ballNode || !gunNode) {
			return;
		}
		_ballNode = ballNode;
		ballNode->RemoveFromParent();
		gunNode->AddChild(ballNode);
		if (auto body = ballNode->FindBehaviour<PhysicsBodyBehaviour>()) {
			body->SetFixed(true);
		}
	}

	void GameControllerBehaviour::DetachBallFromGun() {
		auto ballNode = _ballNode.lock();
		auto rootNode = _fieldNode.lock();
		if (!ballNode || !rootNode) {
			return;
		}
		auto worldPos = Utils::GetWorldPos(ballNode);
		ballNode->RemoveFromParent();
		rootNode->AddChild(ballNode);
		Utils::SetLocalPosToWorld(ballNode, worldPos);
		if (auto body = ballNode->FindBehaviour<PhysicsBodyBehaviour>()) {
			body->SetFixed(false);
		}
	}
} // namespace BallGame1
