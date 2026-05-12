#include "Engine/Editor/PhysicsVisualizer.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/AttractionField.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/VectorArrowShape.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

	constexpr float kMinArrowLengthSq = 1e-1f;
	constexpr float kVelocityInternalScale = 0.1f;
	constexpr float kAccelInternalScale = 0.001f;
	constexpr float kFieldInternalScale = 0.00025f;
	constexpr float kMinFieldSpacingPx = 8.f;
	constexpr float kSqrt3Half = 0.8660254037844386f;
	constexpr std::size_t kMaxFieldSamples = 16384;

	sf::Color LerpRgb(const sf::Color& a, const sf::Color& b, float t) {
		t = std::clamp(t, 0.f, 1.f);
		const auto lerpCh = [t](std::uint8_t x, std::uint8_t y) {
			return static_cast<std::uint8_t>(std::round(std::lerp(static_cast<float>(x), static_cast<float>(y), t)));
		};
		return sf::Color(lerpCh(a.r, b.r), lerpCh(a.g, b.g), lerpCh(a.b, b.b), lerpCh(a.a, b.a));
	}

	sf::Color FieldPaletteColor(
	    float forceLen, float span, const sf::Color& weak, const sf::Color& mid, const sf::Color& strong) {
		float t = 0.f;
		if (span > 0.f && std::isfinite(span)) {
			t = std::clamp(forceLen / span, 0.f, 1.f);
		}
		else if (forceLen > 0.f) {
			t = 1.f;
		}
		if (t <= 0.5f) {
			return LerpRgb(weak, mid, t * 2.f);
		}
		return LerpRgb(mid, strong, (t - 0.5f) * 2.f);
	}

	float SoftCompressArrowLength(float len, float softCapWorld, float compressBlend) {
		if (len <= 0.f || !std::isfinite(len)) {
			return len;
		}
		if (softCapWorld <= 0.f || compressBlend <= 0.f) {
			return len;
		}
		const float b = std::clamp(compressBlend, 0.f, 1.f);
		const float saturated = softCapWorld * len / (len + softCapWorld);
		return std::lerp(len, saturated, b);
	}

	std::size_t EstimateGridSampleCount(unsigned w, unsigned h, float stepPx, Engine::FieldGridLayout layout) {
		if (w == 0 || h == 0 || stepPx < 1.f) {
			return 0;
		}
		if (layout == Engine::FieldGridLayout::Square) {
			const std::size_t nx = static_cast<std::size_t>(std::ceil(static_cast<float>(w) / stepPx)) + 1;
			const std::size_t ny = static_cast<std::size_t>(std::ceil(static_cast<float>(h) / stepPx)) + 1;
			return nx * ny;
		}
		const float dy = stepPx * kSqrt3Half;
		if (dy < 1.f) {
			return 0;
		}
		const std::size_t ny = static_cast<std::size_t>(std::ceil(static_cast<float>(h) / dy)) + 1;
		const std::size_t nx = static_cast<std::size_t>(std::ceil(static_cast<float>(w) / stepPx)) + 3;
		return nx * ny;
	}

} // namespace

namespace Engine {

	void PhysicsVisualizer::Draw(sf::RenderWindow& window) const {
		const bool needBodies = _isVelocityVisible || _isForceVisible;
		const bool needField = _isAttractionFieldVisible;
		if (!needBodies && !needField) {
			return;
		}
		const auto proc = MainContext::GetInstance().GetPhysicsProcessor();
		if (!proc) {
			return;
		}

		if (needBodies) {
			for (const auto& w : proc->GetAllBodies()) {
				const auto body = w.lock();
				if (!body) {
					continue;
				}
				const auto node = body->GetNode();
				if (!node || !node->IsEnabled() || !node->IsVisible()) {
					continue;
				}
				const sf::Vector2f origin = Utils::GetWorldPos(node);
				if (_isVelocityVisible) {
					const sf::Vector2f delta = body->GetVelocity() * _velocityScale * kVelocityInternalScale;
					if (delta.lengthSquared() >= kMinArrowLengthSq) {
						VectorArrowShape arrow(origin, origin + delta, _velocityColor);
						arrow.draw(window, sf::RenderStates::Default);
					}
				}
				if (_isForceVisible) {
					const sf::Vector2f delta =
					    proc->EvaluateExternalForces(body.get()) * _forceScale * kAccelInternalScale;
					if (delta.lengthSquared() >= kMinArrowLengthSq) {
						VectorArrowShape arrow(origin, origin + delta, _forceColor);
						arrow.draw(window, sf::RenderStates::Default);
					}
				}
			}
		}

		if (needField) {
			DrawAttractionFieldOverlay(window, *proc);
		}
	}

	void PhysicsVisualizer::DrawAttractionFieldOverlay(sf::RenderWindow& window, const PhysicsProcessor& proc) const {
		const auto field = proc.GetAttractionField();
		if (!field) {
			return;
		}
		const sf::View view = window.getView();
		const sf::Vector2u sizeU = window.getSize();
		const unsigned w = sizeU.x;
		const unsigned h = sizeU.y;
		if (w == 0 || h == 0) {
			return;
		}

		float stepPx = std::max(_fieldSpacingPx, kMinFieldSpacingPx);
		std::size_t est = EstimateGridSampleCount(w, h, stepPx, _fieldGridLayout);
		while (est > kMaxFieldSamples && stepPx < static_cast<float>(std::max(w, h) * 2)) {
			stepPx *= std::sqrt(static_cast<float>(est) / static_cast<float>(kMaxFieldSamples));
			est = EstimateGridSampleCount(w, h, stepPx, _fieldGridLayout);
		}

		const auto emitFieldArrow = [&](float px, float py) {
			const sf::Vector2i pix{static_cast<int>(std::lround(px)), static_cast<int>(std::lround(py))};
			if (pix.x < 0 || pix.y < 0 || static_cast<unsigned>(pix.x) >= w || static_cast<unsigned>(pix.y) >= h) {
				return;
			}
			const sf::Vector2f world = window.mapPixelToCoords(pix, view);
			if (Utils::IsNan(world)) {
				return;
			}
			const sf::Vector2f f = field->EvaluateForce(world, _fieldProbeGroups);
			sf::Vector2f delta = f * _fieldArrowScale * kFieldInternalScale;
			const float lenSq = delta.lengthSquared();
			if (lenSq < kMinArrowLengthSq) {
				return;
			}
			const float len = std::sqrt(lenSq);
			const float lenOut = SoftCompressArrowLength(len, _fieldArrowLengthSoftCap, _fieldArrowLengthCompress);
			delta *= lenOut / len;
			const float flen = f.length();
			const sf::Color col =
			    FieldPaletteColor(flen, _fieldPaletteSpan, _fieldPaletteWeak, _fieldPaletteMid, _fieldPaletteStrong);
			VectorArrowShape arrow(world, world + delta, col);
			arrow.draw(window, sf::RenderStates::Default);
		};

		if (_fieldGridLayout == FieldGridLayout::Square) {
			for (float py = 0.f; py < static_cast<float>(h); py += stepPx) {
				for (float px = 0.f; px < static_cast<float>(w); px += stepPx) {
					emitFieldArrow(px, py);
				}
			}
		}
		else {
			const float dy = stepPx * kSqrt3Half;
			if (dy < 1.f) {
				return;
			}
			int row = 0;
			for (float py = 0.f; py < static_cast<float>(h); py += dy, ++row) {
				const float x0 = ((row & 1) != 0) ? (stepPx * 0.5f) : 0.f;
				for (float px = x0; px < static_cast<float>(w); px += stepPx) {
					emitFieldArrow(px, py);
				}
			}
		}
	}

	bool PhysicsVisualizer::IsVelocityVisible() const {
		return _isVelocityVisible;
	}

	void PhysicsVisualizer::SetVelocityVisible(bool visible) {
		_isVelocityVisible = visible;
	}

	bool PhysicsVisualizer::IsForceVisible() const {
		return _isForceVisible;
	}

	void PhysicsVisualizer::SetForceVisible(bool visible) {
		_isForceVisible = visible;
	}

	float PhysicsVisualizer::GetVelocityScale() const {
		return _velocityScale;
	}

	void PhysicsVisualizer::SetVelocityScale(float scale) {
		_velocityScale = scale;
	}

	float PhysicsVisualizer::GetForceScale() const {
		return _forceScale;
	}

	void PhysicsVisualizer::SetForceScale(float scale) {
		_forceScale = scale;
	}

	sf::Color PhysicsVisualizer::GetVelocityColor() const {
		return _velocityColor;
	}

	void PhysicsVisualizer::SetVelocityColor(sf::Color color) {
		_velocityColor = color;
	}

	sf::Color PhysicsVisualizer::GetForceColor() const {
		return _forceColor;
	}

	void PhysicsVisualizer::SetForceColor(sf::Color color) {
		_forceColor = color;
	}

	bool PhysicsVisualizer::IsAttractionFieldVisible() const {
		return _isAttractionFieldVisible;
	}

	void PhysicsVisualizer::SetAttractionFieldVisible(bool visible) {
		_isAttractionFieldVisible = visible;
	}

	FieldGridLayout PhysicsVisualizer::GetAttractionFieldGridLayout() const {
		return _fieldGridLayout;
	}

	void PhysicsVisualizer::SetAttractionFieldGridLayout(FieldGridLayout layout) {
		_fieldGridLayout = layout;
	}

	float PhysicsVisualizer::GetAttractionFieldSpacingPx() const {
		return _fieldSpacingPx;
	}

	void PhysicsVisualizer::SetAttractionFieldSpacingPx(float spacingPx) {
		_fieldSpacingPx = std::max(kMinFieldSpacingPx, spacingPx);
	}

	float PhysicsVisualizer::GetAttractionFieldArrowScale() const {
		return _fieldArrowScale;
	}

	void PhysicsVisualizer::SetAttractionFieldArrowScale(float scale) {
		_fieldArrowScale = std::max(1e-6f, scale);
	}

	float PhysicsVisualizer::GetAttractionFieldArrowLengthSoftCap() const {
		return _fieldArrowLengthSoftCap;
	}

	void PhysicsVisualizer::SetAttractionFieldArrowLengthSoftCap(float worldLength) {
		_fieldArrowLengthSoftCap = std::max(0.f, worldLength);
	}

	float PhysicsVisualizer::GetAttractionFieldArrowLengthCompress() const {
		return _fieldArrowLengthCompress;
	}

	void PhysicsVisualizer::SetAttractionFieldArrowLengthCompress(float amount) {
		_fieldArrowLengthCompress = std::clamp(amount, 0.f, 1.f);
	}

	float PhysicsVisualizer::GetAttractionFieldPaletteSpan() const {
		return _fieldPaletteSpan;
	}

	void PhysicsVisualizer::SetAttractionFieldPaletteSpan(float span) {
		_fieldPaletteSpan = std::max(0.f, span);
	}

	sf::Color PhysicsVisualizer::GetAttractionFieldPaletteWeak() const {
		return _fieldPaletteWeak;
	}

	void PhysicsVisualizer::SetAttractionFieldPaletteWeak(sf::Color color) {
		_fieldPaletteWeak = color;
	}

	sf::Color PhysicsVisualizer::GetAttractionFieldPaletteMid() const {
		return _fieldPaletteMid;
	}

	void PhysicsVisualizer::SetAttractionFieldPaletteMid(sf::Color color) {
		_fieldPaletteMid = color;
	}

	sf::Color PhysicsVisualizer::GetAttractionFieldPaletteStrong() const {
		return _fieldPaletteStrong;
	}

	void PhysicsVisualizer::SetAttractionFieldPaletteStrong(sf::Color color) {
		_fieldPaletteStrong = color;
	}

	const PhysicsBodyBehaviour::GroupSet& PhysicsVisualizer::GetAttractionFieldProbeGroups() const {
		return _fieldProbeGroups;
	}

	void PhysicsVisualizer::SetAttractionFieldProbeGroups(const PhysicsBodyBehaviour::GroupSet& groups) {
		_fieldProbeGroups = groups;
	}

} // namespace Engine
