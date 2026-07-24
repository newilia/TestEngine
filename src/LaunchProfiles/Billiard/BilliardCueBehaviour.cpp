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

	void BilliardCueBehaviour::PositionOn(const RefWrapper<SceneNode>& anchor) {
		_anchorRef = anchor;
		const auto anchorNode = anchor.Get();
		const auto cueNode = GetNode();
		if (!anchorNode || !cueNode) {
			return;
		}
		Utils::SetLocalPosToWorld(cueNode, Utils::GetWorldPos(anchorNode));
	}

	void BilliardCueBehaviour::SetRotation(sf::Angle angle) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalRotation(angle);
		}
	}

	void BilliardCueBehaviour::MoveLongitudinal(float delta) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalPosition(cueNode->GetLocalPosition() + sf::Vector2f{delta, 0.f});
		}
	}

	void BilliardCueBehaviour::MoveLateral(float delta) {
		if (auto cueNode = GetNode()) {
			cueNode->SetLocalPosition(cueNode->GetLocalPosition() + sf::Vector2f{0.f, delta});
		}
	}

} // namespace Billiard
