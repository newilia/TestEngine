#include "Engine/Editor/EditorSceneGrid.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace {

	constexpr ImU32 kMinorGridLineColor = IM_COL32(192, 192, 192, 64);
	constexpr ImU32 kMajorGridLineColor = IM_COL32(192, 192, 192, 64);
	constexpr ImU32 kOriginAxisLineColor = IM_COL32(192, 192, 192, 64);
	constexpr float kMinorLineThicknessPx = 1.f;
	constexpr float kMajorLineThicknessPx = 2.f;
	constexpr float kOriginLineThicknessPx = 3.f;
	constexpr int kDefaultSize = 200;
	constexpr int kDefaultBasis = 10;
	constexpr bool kDefaultVisible = true;
	constexpr bool kDefaultSnap = false;
	constexpr float kLineEpsilon = 1e-4f;
	constexpr float kMinCellSizePx = 30.f;
	constexpr int kMaxLodLevel = 64;

	[[nodiscard]] float ComputePixelsPerWorld(const sf::RenderWindow& window, const sf::View& view) {
		const sf::Vector2f viewSize = view.getSize();
		if (viewSize.x <= 0.f || viewSize.y <= 0.f) {
			return 0.f;
		}
		const auto windowSize = window.getSize();
		const float pxPerWorldX = static_cast<float>(windowSize.x) / viewSize.x;
		const float pxPerWorldY = static_cast<float>(windowSize.y) / viewSize.y;
		return std::min(pxPerWorldX, pxPerWorldY);
	}

	float FirstGridLine(float minBound, float step) {
		if (step <= 0.f) {
			return minBound;
		}
		return std::ceil(minBound / step - kLineEpsilon) * step;
	}

	bool IsOnMajorLine(float coord, float majorStep) {
		if (majorStep <= 0.f) {
			return false;
		}
		const float n = std::round(coord / majorStep);
		return std::abs(coord - n * majorStep) <= kLineEpsilon * std::max(1.f, majorStep);
	}

	void DrawVerticalLine(ImDrawList* drawList, const sf::RenderWindow& window, float worldX, float worldTop,
	    float worldBottom, ImU32 color, float thicknessPx) {
		const sf::Vector2i topPx = Utils::MapWorldToWindowPixel(window, {worldX, worldTop});
		const sf::Vector2i bottomPx = Utils::MapWorldToWindowPixel(window, {worldX, worldBottom});
		drawList->AddLine(ImVec2(static_cast<float>(topPx.x), static_cast<float>(topPx.y)),
		    ImVec2(static_cast<float>(bottomPx.x), static_cast<float>(bottomPx.y)), color, thicknessPx);
	}

	void DrawHorizontalLine(ImDrawList* drawList, const sf::RenderWindow& window, float worldY, float worldLeft,
	    float worldRight, ImU32 color, float thicknessPx) {
		const sf::Vector2i leftPx = Utils::MapWorldToWindowPixel(window, {worldLeft, worldY});
		const sf::Vector2i rightPx = Utils::MapWorldToWindowPixel(window, {worldRight, worldY});
		drawList->AddLine(ImVec2(static_cast<float>(leftPx.x), static_cast<float>(leftPx.y)),
		    ImVec2(static_cast<float>(rightPx.x), static_cast<float>(rightPx.y)), color, thicknessPx);
	}

	[[nodiscard]] bool IsOnThickGridLine(float coord, float finestStep, int basis, float viewSpan) {
		float thickStep = finestStep * static_cast<float>(basis);
		while (thickStep <= viewSpan + kLineEpsilon) {
			if (IsOnMajorLine(coord, thickStep)) {
				return true;
			}
			thickStep *= static_cast<float>(basis);
		}
		return false;
	}

	void DrawGridLines(ImDrawList* drawList, const sf::RenderWindow& window, float worldLeft, float worldRight,
	    float worldTop, float worldBottom, float step, ImU32 color, float thicknessPx, float finestStep, int basis,
	    float viewSpan, bool skipThickMultiples) {
		if (step <= 0.f || drawList == nullptr) {
			return;
		}
		for (float x = FirstGridLine(worldLeft, step); x <= worldRight + kLineEpsilon; x += step) {
			if (skipThickMultiples && IsOnThickGridLine(x, finestStep, basis, viewSpan)) {
				continue;
			}
			DrawVerticalLine(drawList, window, x, worldTop, worldBottom, color, thicknessPx);
		}
		for (float y = FirstGridLine(worldTop, step); y <= worldBottom + kLineEpsilon; y += step) {
			if (skipThickMultiples && IsOnThickGridLine(y, finestStep, basis, viewSpan)) {
				continue;
			}
			DrawHorizontalLine(drawList, window, y, worldLeft, worldRight, color, thicknessPx);
		}
	}

} // namespace

namespace Engine {

	EditorSceneGrid::EditorSceneGrid() {
		ResetToDefaults();
	}

	void EditorSceneGrid::ResetToDefaults() {
		_isVisible = kDefaultVisible;
		_snapToGrid = kDefaultSnap;
		_size = std::clamp(kDefaultSize, kMinSize, kMaxSize);
		_basis = std::clamp(kDefaultBasis, kMinBasis, kMaxBasis);
	}

	bool EditorSceneGrid::IsVisible() const {
		return _isVisible;
	}

	bool& EditorSceneGrid::VisibleMutable() {
		return _isVisible;
	}

	bool EditorSceneGrid::IsSnapEnabled() const {
		return _snapToGrid;
	}

	bool& EditorSceneGrid::SnapEnabledMutable() {
		return _snapToGrid;
	}

	int EditorSceneGrid::GetSize() const {
		return _size;
	}

	int EditorSceneGrid::GetBasis() const {
		return _basis;
	}

	int& EditorSceneGrid::SizeMutable() {
		return _size;
	}

	int& EditorSceneGrid::BasisMutable() {
		return _basis;
	}

	float EditorSceneGrid::GetFinestStep() const {
		return static_cast<float>(_size) / static_cast<float>(_basis);
	}

	int EditorSceneGrid::ComputeLodLevel(float pixelsPerWorld) const {
		if (pixelsPerWorld <= 0.f) {
			return 0;
		}
		float step = GetFinestStep();
		int level = 0;
		while (step * pixelsPerWorld < kMinCellSizePx && level < kMaxLodLevel) {
			step *= static_cast<float>(_basis);
			++level;
		}
		return level;
	}

	float EditorSceneGrid::GetFinestStepAtLod(int lodLevel) const {
		return GetFinestStep() * std::pow(static_cast<float>(_basis), static_cast<float>(lodLevel));
	}

	float EditorSceneGrid::GetSnapStep(float pixelsPerWorld) const {
		return GetFinestStepAtLod(ComputeLodLevel(pixelsPerWorld));
	}

	sf::Vector2f EditorSceneGrid::SnapWorldPoint(sf::Vector2f world) const {
		if (!_snapToGrid) {
			return world;
		}
		if (!_isVisible) {
			return world;
		}
		float pixelsPerWorld = 0.f;
		if (const auto window = MainContext::GetInstance().GetMainWindow()) {
			pixelsPerWorld = ComputePixelsPerWorld(*window, window->getView());
		}
		const float step = GetSnapStep(pixelsPerWorld);
		if (step <= 0.f) {
			return world;
		}
		return {std::round(world.x / step) * step, std::round(world.y / step) * step};
	}

	void EditorSceneGrid::Draw(sf::RenderWindow& window) const {
		if (!_isVisible) {
			return;
		}

		const sf::View& view = window.getView();
		const sf::Vector2f center = view.getCenter();
		const sf::Vector2f halfSize = view.getSize() * 0.5f;
		const float worldLeft = center.x - halfSize.x;
		const float worldRight = center.x + halfSize.x;
		const float worldTop = center.y - halfSize.y;
		const float worldBottom = center.y + halfSize.y;

		const float pixelsPerWorld = ComputePixelsPerWorld(window, view);
		const int lodLevel = ComputeLodLevel(pixelsPerWorld);
		const float finestStep = GetFinestStepAtLod(lodLevel);
		const float viewSpan = std::max(worldRight - worldLeft, worldBottom - worldTop);

		ImDrawList* const drawList = ImGui::GetBackgroundDrawList();

		DrawGridLines(drawList, window, worldLeft, worldRight, worldTop, worldBottom, finestStep, kMinorGridLineColor,
		    kMinorLineThicknessPx, finestStep, _basis, viewSpan, true);

		float thickStep = finestStep * static_cast<float>(_basis);
		while (thickStep <= viewSpan + kLineEpsilon) {
			DrawGridLines(drawList, window, worldLeft, worldRight, worldTop, worldBottom, thickStep,
			    kMajorGridLineColor, kMajorLineThicknessPx, finestStep, _basis, viewSpan, false);
			thickStep *= static_cast<float>(_basis);
		}
		if (0.f >= worldLeft - kLineEpsilon && 0.f <= worldRight + kLineEpsilon) {
			DrawVerticalLine(
			    drawList, window, 0.f, worldTop, worldBottom, kOriginAxisLineColor, kOriginLineThicknessPx);
		}
		if (0.f >= worldTop - kLineEpsilon && 0.f <= worldBottom + kLineEpsilon) {
			DrawHorizontalLine(
			    drawList, window, 0.f, worldLeft, worldRight, kOriginAxisLineColor, kOriginLineThicknessPx);
		}
	}

} // namespace Engine
