#include "BilliardBallAimDisplayBehaviour.h"

#include "BilliardBallAimDisplayBehaviour.generated.hpp"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <algorithm>
#include <cmath>

namespace Billiard {

	void BilliardBallAimDisplayBehaviour::OnEvent(const sf::Event& event) {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return;
		}
		const auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
			return Utils::MapWindowPixelToWorld(*window, pixel);
		};

		if (const auto* clicked = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (clicked->button == sf::Mouse::Button::Left) {
				_isPointerDown = true;
				TrySetAimPoint(toWorld(clicked->position));
			}
		}
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				_isPointerDown = false;
			}
		}
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			if (_isPointerDown) {
				TrySetAimPoint(toWorld(moved->position));
			}
		}
	}

	void BilliardBallAimDisplayBehaviour::Show() {
		if (auto node = GetNode()) {
			node->SetVisible(true);
		}
		UpdateAimPointPosition();
	}

	void BilliardBallAimDisplayBehaviour::Hide() {
		if (auto node = GetNode()) {
			node->SetVisible(false);
		}
	}

	void BilliardBallAimDisplayBehaviour::UpdateAimPointPosition() {
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

	void BilliardBallAimDisplayBehaviour::TrySetAimPoint(const sf::Vector2f& worldPoint) {
		auto area = _circleArea.Get();
		auto aimPointNode = _aimPointNode.Get();
		if (!area) {
			return;
		}
		if (!area->HitTest(worldPoint)) {
			return;
		}
		const float radius = area->GetRadius();
		if (radius <= 0.f) {
			return;
		}
		_aimPoint = (worldPoint - Utils::GetWorldPos(area->GetNode())) / radius;
		UpdateAimPointPosition();
		_aimPointChangedSignal.Emit(_aimPoint);
	}

	Signal<sf::Vector2f>& BilliardBallAimDisplayBehaviour::GetAimPointChangedSignal() {
		return _aimPointChangedSignal;
	}

} // namespace Billiard
