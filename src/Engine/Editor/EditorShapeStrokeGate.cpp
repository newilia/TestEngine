#include "Engine/Editor/EditorShapeStrokeGate.h"

#include "Engine/Editor/EditorNodePick.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

namespace {

	constexpr int kDragThresholdPx = 6;
	constexpr int kDragThresholdPx2 = kDragThresholdPx * kDragThresholdPx;

} // namespace

bool EditorShapeStrokeGate::IsDragThresholdExceeded(const sf::Vector2i from, const sf::Vector2i to) {
	const sf::Vector2i delta = to - from;
	return delta.x * delta.x + delta.y * delta.y >= kDragThresholdPx2;
}

bool EditorShapeStrokeGate::ProcessEvent(
    const sf::Event& event, const OnStrokeStarted& onStrokeStarted, const OnClick& onClick) {
	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			_pending = true;
			_pressPixel = pressed->position;
			_pressWorld = EditorNodePick::MapWindowPixelToWorld(pressed->position);
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			_pending = true;
			_pressPixel = touch->position;
			_pressWorld = EditorNodePick::MapWindowPixelToWorld(touch->position);
			return true;
		}
	}

	if (!_pending) {
		return false;
	}

	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		if (IsDragThresholdExceeded(_pressPixel, moved->position)) {
			_pending = false;
			if (onStrokeStarted) {
				onStrokeStarted(_pressWorld, _pressPixel);
			}
		}
		return true;
	}
	if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
		if (tm->finger == 0 && IsDragThresholdExceeded(_pressPixel, tm->position)) {
			_pending = false;
			if (onStrokeStarted) {
				onStrokeStarted(_pressWorld, _pressPixel);
			}
		}
		return true;
	}
	if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (released->button == sf::Mouse::Button::Left) {
			const bool wasPending = _pending;
			_pending = false;
			if (wasPending && onClick) {
				onClick(EditorNodePick::MapWindowPixelToWorld(released->position), released->position,
				    EditorNodePick::IsMultiSelectModifierPressed());
			}
			return true;
		}
	}
	if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
		if (ended->finger == 0) {
			const bool wasPending = _pending;
			_pending = false;
			if (wasPending && onClick) {
				onClick(EditorNodePick::MapWindowPixelToWorld(ended->position), ended->position, false);
			}
			return true;
		}
	}

	return false;
}
