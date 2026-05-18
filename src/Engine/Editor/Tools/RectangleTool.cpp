#include "Engine/Editor/Tools/RectangleTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <imgui.h>

#include <algorithm>

namespace {

	constexpr float kMinWorldSize = 1.f;
	constexpr float kOverlayLineThickness = 3.f;

	sf::Vector2f ToWorldPixel(sf::Vector2i pixel) {
		if (auto* mainWindow = Engine::MainContext::GetInstance().GetMainWindow()) {
			return Utils::MapWindowPixelToWorld(*mainWindow, pixel);
		}
		return {};
	}

	sf::Vector2f SnapWorld(sf::Vector2f world) {
		return Engine::Editor::GetInstance().GetEditorSceneGrid().SnapWorldPoint(world);
	}

} // namespace

RectangleTool::RectangleTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

void RectangleTool::BeginStroke(const sf::Vector2f& world) {
	_isDrawing = true;
	_startWorld = SnapWorld(world);
	_cursorWorld = _startWorld;
}

void RectangleTool::FinalizeStroke() {
	const float minX = std::min(_startWorld.x, _cursorWorld.x);
	const float maxX = std::max(_startWorld.x, _cursorWorld.x);
	const float minY = std::min(_startWorld.y, _cursorWorld.y);
	const float maxY = std::max(_startWorld.y, _cursorWorld.y);
	const sf::Vector2f size(maxX - minX, maxY - minY);
	if (size.x < kMinWorldSize || size.y < kMinWorldSize) {
		return;
	}
	const sf::Vector2f centerWorld((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);

	auto parent = Engine::Editor::GetInstance().GetSelectedNode();
	if (!parent) {
		if (const auto scene = Engine::MainContext::GetInstance().GetScene()) {
			parent = scene->GetRoot();
		}
	}
	if (!parent) {
		return;
	}

	auto node = SceneNode::Create();
	node->SetName("Rectangle");
	auto visual = std::make_shared<RectangleShapeVisual>();
	visual->SetSize(size);
	visual->SetOrigin(size * 0.5f);
	node->SetVisual(std::move(visual));
	Utils::SetLocalPosToWorld(node, centerWorld);
	if (_isAttachPhysicsBody) {
		node->RequireBehaviour<PhysicsBodyBehaviour>();
	}
	parent->AddChild(node);
}

void RectangleTool::EndStroke() {
	if (!_isDrawing) {
		return;
	}
	_isDrawing = false;
	FinalizeStroke();
}

bool RectangleTool::ProcessEvent(const sf::Event& event) {
	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			BeginStroke(ToWorldPixel(pressed->position));
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			BeginStroke(ToWorldPixel(touch->position));
			return true;
		}
	}

	if (_isDrawing) {
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			_cursorWorld = SnapWorld(ToWorldPixel(moved->position));
			return true;
		}
		if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
			if (tm->finger == 0) {
				_cursorWorld = SnapWorld(ToWorldPixel(tm->position));
				return true;
			}
		}
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				_cursorWorld = SnapWorld(ToWorldPixel(released->position));
				EndStroke();
				return true;
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				_cursorWorld = SnapWorld(ToWorldPixel(ended->position));
				EndStroke();
				return true;
			}
		}
	}

	return false;
}

void RectangleTool::DrawOverlay(sf::RenderWindow& window) {
	if (!_isDrawing) {
		return;
	}

	const sf::Vector2i startPx = Utils::MapWorldToWindowPixel(window, _startWorld);
	const sf::Vector2i cursorPx = Utils::MapWorldToWindowPixel(window, _cursorWorld);
	const ImVec2 min(
	    static_cast<float>(std::min(startPx.x, cursorPx.x)), static_cast<float>(std::min(startPx.y, cursorPx.y)));
	const ImVec2 max(
	    static_cast<float>(std::max(startPx.x, cursorPx.x)), static_cast<float>(std::max(startPx.y, cursorPx.y)));

	ImGui::GetForegroundDrawList()->AddRect(
	    min, max, IM_COL32(255, 255, 255, 255), 0.f, ImDrawFlags_None, kOverlayLineThickness);
}

void RectangleTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Drag corner to corner");
	ImGui::Checkbox("Attach physical body behaviour", &_isAttachPhysicsBody);
}
