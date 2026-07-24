#include "BilliardBallAimDisplayBehaviour.h"

#include "BilliardBallAimDisplayBehaviour.generated.hpp"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

#include <algorithm>
#include <cmath>

namespace Billiard {

	void BilliardBallAimDisplayBehaviour::OnUpdate(const sf::Time& /*dt*/) {
		if (_isVisible) {
			UpdateAimPointPosition();
		}
	}

	void BilliardBallAimDisplayBehaviour::Show() {
		_isVisible = true;
		if (auto aimPointNode = _aimPoint.Get()) {
			aimPointNode->SetVisible(true);
		}
		UpdateAimPointPosition();
	}

	void BilliardBallAimDisplayBehaviour::Hide() {
		_isVisible = false;
		if (auto aimPointNode = _aimPoint.Get()) {
			aimPointNode->SetVisible(false);
		}
	}

	void BilliardBallAimDisplayBehaviour::SetAimPoint(const RefWrapper<SceneNode>& aimPoint, float aimRadius) {
		_aimPoint = aimPoint;
		_aimRadius = aimRadius;
		UpdateAimPointPosition();
	}

	void BilliardBallAimDisplayBehaviour::SetAimOffset(sf::Vector2f offset) {
		offset.x = std::clamp(offset.x, -1.f, 1.f);
		offset.y = std::clamp(offset.y, -1.f, 1.f);
		const float length = std::sqrt(offset.x * offset.x + offset.y * offset.y);
		if (length > 1.f) {
			offset /= length;
		}
		_aimOffset = offset;
		UpdateAimPointPosition();
	}

	sf::Vector2f BilliardBallAimDisplayBehaviour::GetAimOffset() const {
		return _aimOffset;
	}

	void BilliardBallAimDisplayBehaviour::UpdateAimPointPosition() {
		const auto aimPointNode = _aimPoint.Get();
		const auto ownerNode = GetNode();
		if (!aimPointNode || !ownerNode) {
			return;
		}
		const sf::Vector2f worldPos = Utils::GetWorldPos(ownerNode) + _aimOffset * _aimRadius;
		Utils::SetLocalPosToWorld(aimPointNode, worldPos);
	}

} // namespace Billiard
