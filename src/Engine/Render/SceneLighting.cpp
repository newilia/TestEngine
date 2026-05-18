#include "SceneLighting.h"

#include "Engine/Behaviour/PointLightBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace Engine {
	namespace {

		bool IsActiveInSceneHierarchy(const SceneNode& node) {
			const SceneNode* n = &node;
			while (n) {
				if (!n->IsEnabled() || !n->IsVisible()) {
					return false;
				}
				auto p = n->GetParent();
				n = p ? p.get() : nullptr;
			}
			return true;
		}

		float DistSqPointToRect(const sf::Vector2f& p, const sf::FloatRect& r) {
			const float nx = std::clamp(p.x, r.position.x, r.position.x + r.size.x);
			const float ny = std::clamp(p.y, r.position.y, r.position.y + r.size.y);
			const float dx = p.x - nx;
			const float dy = p.y - ny;
			return dx * dx + dy * dy;
		}

		bool IsPointInRectBounds(const sf::Vector2f& p, const sf::FloatRect& r) {
			const float x1 = r.position.x;
			const float y1 = r.position.y;
			const float x2 = r.position.x + r.size.x;
			const float y2 = r.position.y + r.size.y;
			return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
		}

	} // namespace

	bool SceneLighting::IsEnabled() const {
		return _enabled;
	}

	void SceneLighting::SetEnabled(bool enabled) {
		_enabled = enabled;
	}

	float SceneLighting::GetDistanceRangeScale() const {
		return _distanceRangeScale;
	}

	void SceneLighting::SetDistanceRangeScale(float scale) {
		_distanceRangeScale = std::max(scale, 0.01f);
	}

	float SceneLighting::GetGlobalIntensityScale() const {
		return _globalIntensityScale;
	}

	void SceneLighting::SetGlobalIntensityScale(float scale) {
		_globalIntensityScale = std::max(scale, 0.f);
	}

	LightingBlendMode SceneLighting::GetBlendMode() const {
		return _blendMode;
	}

	void SceneLighting::SetBlendMode(LightingBlendMode mode) {
		_blendMode = mode;
	}

	const std::vector<GpuPointLight>& SceneLighting::GetLights() const {
		return _lights;
	}

	void SceneLighting::RegisterPointLight(std::shared_ptr<PointLightBehaviour> light) {
		PointLightBehaviour* raw = light.get();
		for (const auto& w : _lightSources) {
			auto locked = w.lock();
			if (locked && locked.get() == raw) {
				return;
			}
		}

		_lightSources.emplace_back(std::move(light));
	}

	void SceneLighting::UnregisterPointLight(PointLightBehaviour* light) {
		auto it = std::find_if(
		    _lightSources.begin(), _lightSources.end(), [light](const std::weak_ptr<PointLightBehaviour>& w) {
			    return w.lock().get() == light;
		    });
		if (it != _lightSources.end()) {
			_lightSources.erase(it);
		}
	}

	void SceneLighting::PrepareLights() {
		if (!_enabled) {
			return;
		}

		_lights.clear();
		for (auto it = _lightSources.begin(); it != _lightSources.end();) {
			auto pl = it->lock();
			if (!pl) {
				it = _lightSources.erase(it);
				continue;
			}
			++it;

			auto node = pl->GetNode();
			if (!node || !IsActiveInSceneHierarchy(*node)) {
				continue;
			}
			if (!pl->IsLightEnabled()) {
				continue;
			}

			const sf::Color c = pl->GetLightColor();
			const float intensity = pl->GetIntensity() * _globalIntensityScale;
			GpuPointLight light{};
			light.position = Utils::GetWorldPos(node);
			light.color = sf::Glsl::Vec3(static_cast<float>(c.r) / 255.f * intensity,
			    static_cast<float>(c.g) / 255.f * intensity, static_cast<float>(c.b) / 255.f * intensity);
			light.radius = std::max(pl->GetRadius() * _distanceRangeScale, 1.f);
			light.sourceNode = node.get();
			_lights.push_back(light);
		}
	}

	void SceneLighting::SelectLightsForBounds(const sf::FloatRect& receiverBounds, const SceneNode* excludeLightsOnNode,
	    std::vector<GpuPointLight>& out, std::size_t maxLights) const {
		out.clear();
		if (!_enabled) {
			return;
		}
		if (maxLights == 0 || _lights.empty()) {
			return;
		}

		// Must match the attenuation tail in ShapeLighting.frag (practical influence radius).
		constexpr float kCullRadiusMul = 10000.f;

		_selectScratch.clear();
		_selectScratch.reserve(_lights.size());
		for (std::size_t i = 0; i < _lights.size(); ++i) {
			const GpuPointLight& light = _lights[i];
			if (excludeLightsOnNode && light.sourceNode == excludeLightsOnNode) {
				continue;
			}
			const float radius = std::max(light.radius, 1.f);
			const float cutoff = radius * kCullRadiusMul;
			const float dsqToEdges = DistSqPointToRect(light.position, receiverBounds);
			if (!IsPointInRectBounds(light.position, receiverBounds)) {
				if (dsqToEdges > cutoff * cutoff) {
					continue;
				}
			}
			_selectScratch.emplace_back(dsqToEdges, i);
		}

		if (_selectScratch.empty()) {
			return;
		}

		const std::size_t take = std::min(maxLights, _selectScratch.size());
		if (take < _selectScratch.size()) {
			const auto nth = _selectScratch.begin() + static_cast<std::ptrdiff_t>(take);
			std::nth_element(_selectScratch.begin(), nth, _selectScratch.end(),
			    [](const std::pair<float, std::size_t>& a, const std::pair<float, std::size_t>& b) {
				    return a.first < b.first;
			    });
		}

		out.reserve(take);
		for (std::size_t k = 0; k < take; ++k) {
			out.push_back(_lights[_selectScratch[k].second]);
		}
	}

} // namespace Engine
