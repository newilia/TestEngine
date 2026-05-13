#include "GameControllerBehaviour.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "GameControllerBehaviour.generated.hpp"

#include <cmath>

namespace BallGame1 {
	void GameControllerBehaviour::OnInit() {
		EventHandlerBehaviourBase::OnInit();
	}

	void GameControllerBehaviour::OnDeinit() {
		EventHandlerBehaviourBase::OnDeinit();
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

	void GameControllerBehaviour::SetCreateBallFunc(const std::function<std::shared_ptr<SceneNode>(void)>& func) {
		_createBall = func;
	}

	void GameControllerBehaviour::SetShootVelocity(float vel) {
		_shootVelocity = vel;
	}

	void GameControllerBehaviour::StartNewGame() {
		if (auto ballNode = CreateBallNode()) {
			AttachBallToGun(ballNode);
		}
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
		ballBody->SetVelocity(moveDirection * _shootVelocity);
		DetachBallFromGun();

		if (auto ballNode = CreateBallNode()) {
			AttachBallToGun(ballNode);
		}
	}

	std::shared_ptr<SceneNode> GameControllerBehaviour::CreateBallNode() const {
		if (Verify(_createBall)) {
			return _createBall();
		}
		return nullptr;
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
		if (auto attractive = ballNode->FindBehaviour<AttractiveBehaviour>()) {
			attractive->SetEnabled(false);
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
		if (auto attractive = ballNode->FindBehaviour<AttractiveBehaviour>()) {
			attractive->SetEnabled(true);
		}
	}
} // namespace BallGame1
