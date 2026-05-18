#include "Engine/Editor/Tools/CircleTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/ColorUtils.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorNodePick.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <fmt/format.h>
#include <imgui.h>

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

CircleTool::CircleTool(EditorNodePick::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

void CircleTool::BeginStroke(const sf::Vector2f& world) {
	_isDrawing = true;
	_startWorld = SnapWorld(world);
	_cursorWorld = _startWorld;
}

void CircleTool::FinalizeStroke() {
	const float radius = Utils::Length(_cursorWorld - _startWorld);
	if (radius < kMinWorldSize) {
		return;
	}

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
	node->SetName("Circle");
	auto visual = std::make_shared<CircleShapeVisual>();
	visual->SetRadius(radius);
	const Utils::HsvShapeColors colors = Utils::RandomHsvShapeColors();
	visual->SetFillColor(colors.fill);
	visual->SetOutlineColor(colors.outline);
	visual->SetOutlineThickness(-1.f);
	visual->SetSectorColor(colors.outline);
	node->SetVisual(std::move(visual));
	Utils::SetLocalPosToWorld(node, _startWorld);
	if (_isAttachPhysicsBody) {
		node->RequireBehaviour<PhysicsBodyBehaviour>();
	}
	parent->AddChild(node);
}

void CircleTool::EndStroke() {
	if (!_isDrawing) {
		return;
	}
	_isDrawing = false;
	FinalizeStroke();
}

bool CircleTool::ProcessEvent(const sf::Event& event) {
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

	return _strokeGate.ProcessEvent(
	    event,
	    [this](const sf::Vector2f world, const sf::Vector2i /*pixel*/) {
		    BeginStroke(world);
	    },
	    [this](const sf::Vector2f world, const sf::Vector2i /*pixel*/, const bool isCtrlPressed) {
		    EditorNodePick::ApplyHierarchyPickAtWorld(world, isCtrlPressed, _onSelect);
	    });
}

void CircleTool::DrawOverlay(sf::RenderWindow& window) {
	if (!_isDrawing) {
		return;
	}

	const sf::Vector2i centerPx = Utils::MapWorldToWindowPixel(window, _startWorld);
	const sf::Vector2i edgePx = Utils::MapWorldToWindowPixel(window, _cursorWorld);
	const sf::Vector2f delta(static_cast<float>(edgePx.x - centerPx.x), static_cast<float>(edgePx.y - centerPx.y));
	const float radiusPx = Utils::Length(delta);

	ImGui::GetForegroundDrawList()->AddCircle(ImVec2(static_cast<float>(centerPx.x), static_cast<float>(centerPx.y)),
	    radiusPx, IM_COL32(255, 255, 255, 255), 0, kOverlayLineThickness);
}

void CircleTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Click selects; drag draws. Ctrl+click toggles.");
	ImGui::TextUnformatted("Drag center to edge");
	ImGui::Checkbox("Attach physical body behaviour", &_isAttachPhysicsBody);
}

std::optional<std::string> CircleTool::TryGetCursorOverlayText() const {
	if (!_isDrawing) {
		return std::nullopt;
	}
	const float radius = Utils::Length(_cursorWorld - _startWorld);
	return fmt::format("Radius = {:.1f}", radius);
}
