#include "Engine/Editor/Tools/PolygonTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/ColorUtils.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Visual/ConvexShapeVisual.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <imgui.h>

#include <algorithm>

namespace {

	constexpr float kMinSampleSpacingPx = 5.f;
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

PolygonTool::PolygonTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

void PolygonTool::BeginStroke(const sf::Vector2f& world, const sf::Vector2i& pixel) {
	_isDrawing = true;
	_worldSamples.clear();
	const sf::Vector2f snapped = SnapWorld(world);
	_worldSamples.push_back(snapped);
	_lastSamplePixel = pixel;
	_cursorWorld = snapped;
}

void PolygonTool::TryAppendSample(const sf::Vector2i& pixel, const sf::Vector2f& world) {
	if (!_lastSamplePixel) {
		return;
	}
	const sf::Vector2f delta(
	    static_cast<float>(pixel.x - _lastSamplePixel->x), static_cast<float>(pixel.y - _lastSamplePixel->y));
	if (Utils::Length(delta) >= kMinSampleSpacingPx) {
		_worldSamples.push_back(SnapWorld(world));
		_lastSamplePixel = pixel;
	}
}

void PolygonTool::FinalizeStroke() {
	if (_worldSamples.size() < 3) {
		return;
	}
	std::vector<sf::Vector2f> hull = Utils::ConvexHull2D(_worldSamples);
	if (hull.size() < 3) {
		return;
	}
	float minX = hull[0].x;
	float maxX = hull[0].x;
	float minY = hull[0].y;
	float maxY = hull[0].y;
	for (std::size_t i = 1; i < hull.size(); ++i) {
		const auto& p = hull[i];
		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y);
		maxY = std::max(maxY, p.y);
	}
	const sf::Vector2f centerWorld((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);

	std::vector<sf::Vector2f> localPts;
	localPts.reserve(hull.size());
	for (const auto& p : hull) {
		localPts.push_back(p - centerWorld);
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
	node->SetName("Polygon");
	auto visual = std::make_shared<ConvexShapeVisual>();
	visual->SetPoints(localPts);
	const Utils::HsvShapeColors colors = Utils::RandomHsvShapeColors();
	visual->SetFillColor(colors.fill);
	visual->SetOutlineColor(colors.outline);
	visual->SetOutlineThickness(-1.f);
	node->SetVisual(std::move(visual));
	Utils::SetLocalPosToWorld(node, centerWorld);
	if (_isAttachPhysicsBody) {
		auto body = node->RequireBehaviour<PhysicsBodyBehaviour>();
	}
	parent->AddChild(node);
}

void PolygonTool::EndStroke() {
	if (!_isDrawing) {
		return;
	}
	_isDrawing = false;
	FinalizeStroke();
	_worldSamples.clear();
	_lastSamplePixel.reset();
}

bool PolygonTool::ProcessEvent(const sf::Event& event) {
	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			BeginStroke(ToWorldPixel(pressed->position), pressed->position);
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			BeginStroke(ToWorldPixel(touch->position), touch->position);
			return true;
		}
	}

	if (_isDrawing) {
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			_cursorWorld = SnapWorld(ToWorldPixel(moved->position));
			TryAppendSample(moved->position, _cursorWorld);
			return true;
		}
		if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
			if (tm->finger == 0) {
				_cursorWorld = SnapWorld(ToWorldPixel(tm->position));
				TryAppendSample(tm->position, _cursorWorld);
				return true;
			}
		}
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				EndStroke();
				return true;
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				EndStroke();
				return true;
			}
		}
	}

	return false;
}

void PolygonTool::DrawOverlay(sf::RenderWindow& window) {
	if (!_isDrawing || _worldSamples.empty()) {
		return;
	}

	std::vector<ImVec2> pts;
	pts.reserve(_worldSamples.size() + 1);
	for (const auto& w : _worldSamples) {
		const sf::Vector2i px = Utils::MapWorldToWindowPixel(window, w);
		pts.emplace_back(static_cast<float>(px.x), static_cast<float>(px.y));
	}
	const sf::Vector2i cpx = Utils::MapWorldToWindowPixel(window, _cursorWorld);
	pts.emplace_back(static_cast<float>(cpx.x), static_cast<float>(cpx.y));

	if (pts.size() < 2) {
		return;
	}

	ImGui::GetForegroundDrawList()->AddPolyline(pts.data(), static_cast<int>(pts.size()), IM_COL32(255, 255, 255, 255),
	    ImDrawFlags_None, kOverlayLineThickness);
}

void PolygonTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Draw a convex shape");
	ImGui::Checkbox("Attach physical body behaviour", &_isAttachPhysicsBody);
}
