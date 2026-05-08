#include "SceneLighting.h"

#include "Engine/Behaviour/PointLightBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <algorithm>
#include <cmath>

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

		float SqDistPointToAxisAlignedRect(const sf::Vector2f& p, const sf::FloatRect& r) {
			const float minX = r.position.x;
			const float minY = r.position.y;
			const float maxX = r.position.x + r.size.x;
			const float maxY = r.position.y + r.size.y;
			const float dx = (p.x < minX) ? (minX - p.x) : (p.x > maxX ? (p.x - maxX) : 0.f);
			const float dy = (p.y < minY) ? (minY - p.y) : (p.y > maxY ? (p.y - maxY) : 0.f);
			return dx * dx + dy * dy;
		}

		struct Candidate
		{
			float score{};
			std::size_t index{};
		};

	} // namespace

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
			const float i = pl->GetIntensity();
			GpuPointLight L{};
			L.position = Utils::GetWorldPos(node);
			L.color = sf::Glsl::Vec3(static_cast<float>(c.r) / 255.f * i, static_cast<float>(c.g) / 255.f * i,
			    static_cast<float>(c.b) / 255.f * i);
			L.radius = std::max(pl->GetRadius(), 1.f);
			_lights.push_back(L);
		}
	}

	const std::vector<GpuPointLight>& SceneLighting::GetLights() const {
		return _lights;
	}

	void SceneLighting::SelectLightsForBounds(
	    const sf::FloatRect& receiverBounds, std::vector<GpuPointLight>& out, std::size_t maxLights) const {
		out.clear();
		if (maxLights == 0 || _lights.empty()) {
			return;
		}

		const sf::Vector2f rectCenter = {receiverBounds.position.x + receiverBounds.size.x * 0.5f,
		    receiverBounds.position.y + receiverBounds.size.y * 0.5f};
		const float rectDiagSq =
		    receiverBounds.size.x * receiverBounds.size.x + receiverBounds.size.y * receiverBounds.size.y;

		std::vector<Candidate> candidates;
		candidates.reserve(_lights.size());

		for (std::size_t i = 0; i < _lights.size(); ++i) {
			const GpuPointLight& L = _lights[i];
			const float minSq = SqDistPointToAxisAlignedRect(L.position, receiverBounds);
			const float influence = L.radius + std::sqrt(rectDiagSq) * 0.5f + 8.f;
			if (minSq > influence * influence) {
				continue;
			}
			const float dx = L.position.x - rectCenter.x;
			const float dy = L.position.y - rectCenter.y;
			const float score = dx * dx + dy * dy;
			candidates.push_back({score, i});
		}

		const std::size_t take = std::min(maxLights, candidates.size());
		std::partial_sort(candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(take), candidates.end(),
		    [](const Candidate& a, const Candidate& b) {
			    return a.score < b.score;
		    });

		out.reserve(take);
		for (std::size_t k = 0; k < take; ++k) {
			out.push_back(_lights[candidates[k].index]);
		}
	}

} // namespace Engine
