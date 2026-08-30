#include "CueBallAimWidgetBehaviour.h"

#include "CueBallAimWidgetBehaviour.generated.hpp"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

namespace Billiard {

	void CueBallAimWidgetBehaviour::Show() {
		if (auto node = GetNode()) {
			node->SetVisible(true);
		}
		UpdateAimPointPosition();
	}

	void CueBallAimWidgetBehaviour::Hide() {
		if (auto node = GetNode()) {
			node->SetVisible(false);
		}
	}

	void CueBallAimWidgetBehaviour::ResetAimPoint() {
		_aimPoint = sf::Vector2f();
		UpdateAimPointPosition();
		_aimPointChangedSignal.Emit(_aimPoint);
	}

	void CueBallAimWidgetBehaviour::SetAimPoint(const sf::Vector2f& aimPoint) {
		_aimPoint = aimPoint;
		UpdateAimPointPosition();
	}

	bool CueBallAimWidgetBehaviour::TrySetAimPointFromWorld(const sf::Vector2f& worldPoint) {
		auto area = _circleArea.Get();
		if (!area) {
			return false;
		}
		if (!area->HitTest(worldPoint)) {
			return false;
		}
		const float radius = area->GetRadius();
		if (radius <= 0.f) {
			return false;
		}
		_aimPoint = (worldPoint - Utils::GetWorldPos(area->GetNode())) / radius;
		UpdateAimPointPosition();
		_aimPointChangedSignal.Emit(_aimPoint);
		return true;
	}

	void CueBallAimWidgetBehaviour::UpdateAimPointPosition() {
		const auto aimPointNode = _aimPointNode.Get();
		const auto ownerNode = GetNode();
		const auto circleArea = _circleArea.Get();
		if (!aimPointNode || !ownerNode || !circleArea) {
			return;
		}
		auto radius = circleArea->GetRadius();

		const sf::Vector2f worldPos = Utils::GetWorldPos(ownerNode) + _aimPoint * radius;
		Utils::SetLocalPosToWorld(aimPointNode, worldPos);
	}

	Signal<sf::Vector2f>& CueBallAimWidgetBehaviour::GetAimPointChangedSignal() const {
		return _aimPointChangedSignal;
	}

} // namespace Billiard
