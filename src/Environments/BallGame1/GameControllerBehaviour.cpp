#include "GameControllerBehaviour.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
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
			if (e->button == sf::Mouse::Button::Left) {}
		}
	}

	void GameControllerBehaviour::SetRootNode(const std::weak_ptr<SceneNode>& rootNode) {
		_rootNode = rootNode;
	}

	void GameControllerBehaviour::SetBallNode(const std::weak_ptr<SceneNode>& ballNode) {
		_ballNode = ballNode;
	}

	void GameControllerBehaviour::SetGunNode(const std::weak_ptr<SceneNode>& gunNode) {
		_gunNode = gunNode;
	}

	void GameControllerBehaviour::SetScoreNode(const std::weak_ptr<SceneNode>& scoreNode) {
		_scoreNode = scoreNode;
	}

	void GameControllerBehaviour::StartNewGame() {
		_ballNode = CreateBallNode();
		AttachBallToGun();
	}

	void GameControllerBehaviour::Shoot() {
		auto ballNode = _ballNode.lock();
		auto gunNode = _gunNode.lock();

		if (!ballNode || !gunNode) {
			return;
		}
		auto gunRotation = gunNode->GetLocalTransform()->GetRotation();
		auto ballBody = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
		auto moveDirection = sf::Vector2f(std::cos(gunRotation.asRadians()), std::sin(gunRotation.asRadians()));
		ballBody->SetVelocity(moveDirection * _ballSpeed);
		DetachBallFromGun();

		auto newBallNode = CreateBallNode();
		newBallNode->GetLocalTransform()->SetPosition(gunNode->GetLocalTransform()->GetPosition());
		_ballNode = newBallNode;
		AttachBallToGun();
	}

	std::shared_ptr<SceneNode> GameControllerBehaviour::CreateBallNode() const {
		auto newBallNode = make_shared<SceneNode>();

		auto body = newBallNode->RequireBehaviour<PhysicsBodyBehaviour>();
		body->SetMass(_ballMass);
		body->SetRestitution(_ballRestitution);

		auto visual = newBallNode->RequireVisual<CircleShapeVisual>();
		visual->SetRadius(_ballRadius);
		visual->SetFillColor(_ballColor);
	}

	void GameControllerBehaviour::AttachBallToGun() {
		auto ballNode = _ballNode.lock();
		auto gunNode = _gunNode.lock();
		if (!ballNode || !gunNode) {
			return;
		}
		ballNode->RemoveFromParent();
		gunNode->AddChild(ballNode);
	}

	void GameControllerBehaviour::DetachBallFromGun() {
		auto ballNode = _ballNode.lock();
		auto rootNode = _rootNode.lock();
		if (!ballNode || !rootNode) {
			return;
		}

		ballNode->RemoveFromParent();
		rootNode->AddChild(ballNode);
	}
} // namespace BallGame1
