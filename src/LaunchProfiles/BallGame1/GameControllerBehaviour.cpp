#include "GameControllerBehaviour.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "GameControllerBehaviour.generated.hpp"

#include <cmath>

namespace BallGame1 {
	void GameControllerBehaviour::OnInit() {
		EventHandlerBehaviourBase::OnInit();
		StartNewGame();
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

	void GameControllerBehaviour::StartNewGame() {
		if (auto ballNode = CreateRandomBallNode()) {
			AttachBallToGun(ballNode);
		}
	}

	void GameControllerBehaviour::Shoot() {
		auto ballNode = _ballNode.lock();
		auto gunNode = _gunNodeRef.Get();
		if (ballNode && gunNode) {
			auto gunRotation = gunNode->GetLocalRotation().asRadians();
			auto ballBody = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
			auto moveDirection = sf::Vector2f(std::sin(gunRotation), -std::cos(gunRotation));
			ballBody->SetVelocity(moveDirection * _shootVelocity);
			DetachBallFromGun();
		}

		if (auto ballNode = CreateRandomBallNode()) {
			AttachBallToGun(ballNode);
		}
	}

	std::shared_ptr<SceneNode> GameControllerBehaviour::CreateBallNode(size_t index) const {
		if (index >= _ballAssetRefs.size()) {
			return nullptr;
		}
		if (Verify(_ballAssetRefs[index])) {
			return _ballAssetRefs[index].Get()->GetNode();
		}
		return nullptr;
	}

	std::shared_ptr<SceneNode> GameControllerBehaviour::CreateRandomBallNode() const {
		if (_ballAssetRefs.empty()) {
			return nullptr;
		}
		auto index = rand() % _ballAssetRefs.size();
		return CreateBallNode(index);
	}

	void GameControllerBehaviour::AttachBallToGun(const std::shared_ptr<SceneNode>& ballNode) {
		auto gunNode = _gunNodeRef.Get();
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
		auto rootNode = _fieldNodeRef.Get();
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
