#include "SelectTool.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/Editor.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <imgui.h>

#include <algorithm>

SelectTool::SelectTool(SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

bool SelectTool::ProcessEvent(const sf::Event& event) {
	auto applyPick = [this](const sf::Vector2f& worldPoint, const bool isCtrlPressed) -> bool {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = scene ? scene->FindTopMostNodeAtPoint(worldPoint) : nullptr;
		if (isCtrlPressed && picked) {
			Engine::Editor::GetInstance().ToggleSelectedNode(std::move(picked));
		}
		else {
			_onSelect(std::move(picked));
		}
		return true;
	};

	auto* window = Engine::MainContext::GetInstance().GetMainWindow();
	if (!window) {
		return false;
	}

	auto toWorld = [window](const sf::Vector2i pixel) {
		return Utils::MapWindowPixelToWorld(*window, pixel);
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			_mousePressed = true;
			_isMarqueeSelecting = false;
			_dragStartPixel = pressed->position;
			_dragStartWorld = toWorld(pressed->position);
			_dragCurrentWorld = _dragStartWorld;
			_marqueeMode =
			    (pressed->position.x >= _dragStartPixel.x) ? MarqueeMode::kIntersects : MarqueeMode::kContains;
			_marqueeBaseSelection = Engine::Editor::GetInstance().GetSelectedNodes();
			_marqueeBaseSelectionPtrSet.clear();
			for (const auto& node : _marqueeBaseSelection) {
				if (node) {
					_marqueeBaseSelectionPtrSet.insert(node.get());
				}
			}
			_lastLiveMarqueeEmitPixel.reset();
			_lastLiveMarqueeEmitCtrl.reset();
			_lastLiveMarqueeEmitMode.reset();
			return true;
		}
	}
	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		if (_mousePressed) {
			const sf::Vector2i pixelDelta = moved->position - _dragStartPixel;
			const int dragDistance2 = pixelDelta.x * pixelDelta.x + pixelDelta.y * pixelDelta.y;
			constexpr int kDragThresholdPx = 6;
			constexpr int kDragThresholdPx2 = kDragThresholdPx * kDragThresholdPx;
			if (dragDistance2 >= kDragThresholdPx2) {
				_isMarqueeSelecting = true;
			}
			_dragCurrentWorld = toWorld(moved->position);
			_marqueeMode = (moved->position.x >= _dragStartPixel.x) ? MarqueeMode::kIntersects : MarqueeMode::kContains;
			if (_isMarqueeSelecting) {
				const Scene::RectSelectionMode mode = (_marqueeMode == MarqueeMode::kContains)
				                                          ? Scene::RectSelectionMode::kContains
				                                          : Scene::RectSelectionMode::kIntersects;
				const bool isCtrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
				                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
				const bool marqueeStateUnchanged =
				    _lastLiveMarqueeEmitPixel && _lastLiveMarqueeEmitCtrl && _lastLiveMarqueeEmitMode &&
				    *_lastLiveMarqueeEmitPixel == moved->position && *_lastLiveMarqueeEmitCtrl == isCtrlPressed &&
				    *_lastLiveMarqueeEmitMode == _marqueeMode;
				auto scene = Engine::MainContext::GetInstance().GetScene();
				if (scene && !marqueeStateUnchanged) {
					auto marqueeNodes = scene->FindNodesInRect(CurrentMarqueeRect(), mode);
					Engine::Editor::GetInstance().SetSelectedNodes(
					    BuildLiveMarqueeSelection(marqueeNodes, isCtrlPressed));
					_lastLiveMarqueeEmitPixel = moved->position;
					_lastLiveMarqueeEmitCtrl = isCtrlPressed;
					_lastLiveMarqueeEmitMode = _marqueeMode;
				}
			}
			return true;
		}
	}
	if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (released->button == sf::Mouse::Button::Left && _mousePressed) {
			const bool isCtrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
			                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
			if (_isMarqueeSelecting) {
				auto scene = Engine::MainContext::GetInstance().GetScene();
				if (scene) {
					const Scene::RectSelectionMode mode = (_marqueeMode == MarqueeMode::kContains)
					                                          ? Scene::RectSelectionMode::kContains
					                                          : Scene::RectSelectionMode::kIntersects;
					auto selectedNodes = scene->FindNodesInRect(CurrentMarqueeRect(), mode);
					Engine::Editor::GetInstance().SetSelectedNodes(
					    BuildLiveMarqueeSelection(selectedNodes, isCtrlPressed));
				}
			}
			else {
				(void)applyPick(toWorld(released->position), isCtrlPressed);
			}

			_mousePressed = false;
			_isMarqueeSelecting = false;
			_marqueeBaseSelection.clear();
			_marqueeBaseSelectionPtrSet.clear();
			_lastLiveMarqueeEmitPixel.reset();
			_lastLiveMarqueeEmitCtrl.reset();
			_lastLiveMarqueeEmitMode.reset();
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return applyPick(toWorld(touch->position), false);
		}
	}
	return false;
}

sf::FloatRect SelectTool::CurrentMarqueeRect() const {
	const sf::Vector2f minPoint{
	    std::min(_dragStartWorld.x, _dragCurrentWorld.x), std::min(_dragStartWorld.y, _dragCurrentWorld.y)};
	const sf::Vector2f maxPoint{
	    std::max(_dragStartWorld.x, _dragCurrentWorld.x), std::max(_dragStartWorld.y, _dragCurrentWorld.y)};
	return {minPoint, maxPoint - minPoint};
}

std::vector<std::shared_ptr<SceneNode>> SelectTool::BuildLiveMarqueeSelection(
    const std::vector<std::shared_ptr<SceneNode>>& marqueeNodes, const bool isCtrlPressed) const {
	if (!isCtrlPressed) {
		return marqueeNodes;
	}

	std::vector<std::shared_ptr<SceneNode>> result = _marqueeBaseSelection;
	for (const auto& node : marqueeNodes) {
		if (!node) {
			continue;
		}
		if (!_marqueeBaseSelectionPtrSet.contains(node.get())) {
			result.push_back(node);
		}
	}
	return result;
}

void SelectTool::DrawOverlay(sf::RenderWindow& window) {
	if (!_isMarqueeSelecting) {
		return;
	}
	const sf::FloatRect rect = CurrentMarqueeRect();
	sf::RectangleShape marquee;
	marquee.setPosition(rect.position);
	marquee.setSize(rect.size);
	const sf::Color color =
	    (_marqueeMode == MarqueeMode::kContains) ? sf::Color(255, 110, 80, 180) : sf::Color(80, 170, 255, 180);
	marquee.setFillColor(sf::Color(color.r, color.g, color.b, 45));
	marquee.setOutlineColor(color);
	marquee.setOutlineThickness(1.5f);
	window.draw(marquee);
}

void SelectTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Click selects; Ctrl+Click toggles.");
	ImGui::TextUnformatted("Drag L->R: intersect mode (blue). Drag R->L: contain mode (orange).");
}
