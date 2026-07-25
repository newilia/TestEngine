#include "BilliardCueBehaviour.h"

#include "BilliardCueBehaviour.generated.hpp"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

namespace Billiard {

	void BilliardCueBehaviour::Activate() {
		_isActive = true;
		if (auto body = _bodyRef.Get()) {
			body->SetFixed(false);
		}
	}

	void BilliardCueBehaviour::Deactivate() {
		_isActive = false;
		if (auto body = _bodyRef.Get()) {
			body->SetFixed(true);
		}
	}

	void BilliardCueBehaviour::PositionOnNode(const std::shared_ptr<SceneNode>& node) {
		const auto cueNode = GetNode();
		if (!cueNode) {
			return;
		}
		Utils::SetLocalPosToWorld(cueNode, Utils::GetWorldPos(node));
	}

	void BilliardCueBehaviour::SetRotation(sf::Angle angle) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalRotation(angle);
		}
	}

	void BilliardCueBehaviour::SetLongitudinalPosition(float position) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalOrigin(sf::Vector2f{position, cueNode->GetLocalOrigin().y});
		}
	}

	void BilliardCueBehaviour::MoveLongitudinal(float delta) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalOrigin(cueNode->GetLocalOrigin() + sf::Vector2f{delta, 0.f});
		}
	}

	void BilliardCueBehaviour::SetLateralPosition(float position) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalOrigin(sf::Vector2f{cueNode->GetLocalOrigin().x, position});
		}
	}

	void BilliardCueBehaviour::MoveLateral(float delta) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalOrigin(cueNode->GetLocalOrigin() + sf::Vector2f{0.f, delta});
		}
	}

	void BilliardCueBehaviour::SetVerticalSpin(float spin) {
		_verticalSpin = spin;
	}

} // namespace Billiard
