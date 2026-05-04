#include "Engine/Editor/Tools/PolygonTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
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

} // namespace

PolygonTool::PolygonTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

void PolygonTool::beginStroke(const sf::Vector2f& world, const sf::Vector2i& pixel) {
	_isDrawing = true;
	_worldSamples.clear();
	_worldSamples.push_back(world);
	_lastSamplePixel = pixel;
	_cursorWorld = world;
}

void PolygonTool::tryAppendSample(const sf::Vector2i& pixel, const sf::Vector2f& world) {
	if (!_lastSamplePixel) {
		return;
	}
	const sf::Vector2f delta(static_cast<float>(pixel.x - _lastSamplePixel->x),
	                         static_cast<float>(pixel.y - _lastSamplePixel->y));
	if (Utils::Length(delta) >= kMinSampleSpacingPx) {
		_worldSamples.push_back(world);
		_lastSamplePixel = pixel;
	}
}

void PolygonTool::finalizeStroke() {
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
		parent = Engine::MainContext::GetInstance().GetScene();
	}
	if (!parent) {
		return;
	}

	auto node = std::make_shared<SceneNode>();
	node->SetName("Polygon");
	auto visual = std::make_shared<ConvexShapeVisual>();
	visual->SetPoints(localPts);
	node->SetVisual(std::move(visual));
	parent->AddChild(node);
	Utils::SetWorldPos(node, centerWorld);
	if (_isAttachPhysicsBody) {
		auto body = node->RequireBehaviour<PhysicsBodyBehaviour>();
	}
	/* TODO remove when behaviours init after node attach is fixed */
	{
		auto active = Engine::MainContext::GetInstance().GetScene();
		shared_ptr<SceneNode> root = parent;
		while (auto p = root->GetParent()) {
			root = std::move(p);
		}
		if (active && root == active) {
			node->NotifyLifecycleInitRecursive();
		}
	}
}

void PolygonTool::endStroke() {
	if (!_isDrawing) {
		return;
	}
	_isDrawing = false;
	finalizeStroke();
	_worldSamples.clear();
	_lastSamplePixel.reset();
}

bool PolygonTool::processEvent(const sf::Event& event) {
	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			beginStroke(ToWorldPixel(pressed->position), pressed->position);
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			beginStroke(ToWorldPixel(touch->position), touch->position);
			return true;
		}
	}

	if (_isDrawing) {
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			_cursorWorld = ToWorldPixel(moved->position);
			tryAppendSample(moved->position, _cursorWorld);
			return true;
		}
		if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
			if (tm->finger == 0) {
				_cursorWorld = ToWorldPixel(tm->position);
				tryAppendSample(tm->position, _cursorWorld);
				return true;
			}
		}
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				endStroke();
				return true;
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				endStroke();
				return true;
			}
		}
	}

	return false;
}

void PolygonTool::drawOverlay(sf::RenderWindow& window) {
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

void PolygonTool::drawToolParametersUi() {
	ImGui::TextUnformatted("Draw a convex shape");
	ImGui::Checkbox("Attach physical body behaviour", &_isAttachPhysicsBody);
}
