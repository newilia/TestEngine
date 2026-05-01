#include "AttractionField.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"

#include <cmath>

void AttractionField::Register(const shared_ptr<AttractiveBehaviour>& s) {
	if (!s) {
		return;
	}
	for (const auto& w : _sources) {
		if (const auto p = w.lock()) {
			if (p == s) {
				return;
			}
		}
	}
	_sources.push_back(s);
}

void AttractionField::Unregister(const shared_ptr<AttractiveBehaviour>& s) {
	_sources.remove_if([&s](const std::weak_ptr<AttractiveBehaviour>& w) { return w.lock() == s; });
}

static float EffectiveSourceMass(const RigidBodyBehaviour& rb) {
	if (rb.IsImmovable() || !std::isfinite(rb._mass) || rb._mass <= 0.f) {
		return 1.f;
	}
	return rb._mass;
}

[[nodiscard]] sf::Vector2f
AttractionField::EvaluateAcceleration(const shared_ptr<AttractiveBehaviour>& receiver) const {
	_sources.remove_if([](const std::weak_ptr<AttractiveBehaviour>& w) { return w.expired(); });

	if (!receiver) {
		return {};
	}
	const auto recvNode = receiver->GetNode();
	if (!recvNode) {
		return {};
	}
	const sf::Vector2f posI = recvNode->GetPosGlobal();

	sf::Vector2f acc{};
	for (const auto& w : _sources) {
		const auto other = w.lock();
		if (!other || !other->_isEnabled || other == receiver) {
			continue;
		}
		const auto otherNode = other->GetNode();
		if (!otherNode) {
			continue;
		}
		sf::Vector2f d = otherNode->GetPosGlobal() - posI; // from receiver toward source
		const float d2 = d.x * d.x + d.y * d.y;
		const float e2 = _softeningEps * _softeningEps;
		const float r2 = d2 + e2;
		if (r2 < 1e-8f) {
			continue;
		}
		const float invR = 1.f / std::sqrt(r2);
		const float invR3 = invR * invR * invR; // 1 / (d2+e2)^{3/2}

		if (other->_attraction == 0.f) {
			continue;
		}
		const float t = std::abs(other->_attraction) / 100.f;
		const float mag = std::pow(t, 1.2f);
		// Negative _attraction: pull toward the source; positive: push away. d = posSource - posReceiver.
		const float dir = (other->_attraction < 0.f) ? 1.f : -1.f;
		const float scalar = _globalStrengthScale * mag * dir * invR3;

		float sourceWeight = 1.f;
		if (_useMassCoupling) {
			if (const auto srb = otherNode->FindBehaviour<RigidBodyBehaviour>()) {
				sourceWeight = EffectiveSourceMass(*srb);
			}
		}

		acc += scalar * sourceWeight * d;
	}
	return acc;
}

float AttractionField::GetGlobalStrengthScale() const {
	return _globalStrengthScale;
}

void AttractionField::SetGlobalStrengthScale(float v) {
	_globalStrengthScale = v;
}

bool AttractionField::GetUseMassCoupling() const {
	return _useMassCoupling;
}

void AttractionField::SetUseMassCoupling(bool v) {
	_useMassCoupling = v;
}

float AttractionField::GetSofteningEps() const {
	return _softeningEps;
}

void AttractionField::SetSofteningEps(float e) {
	_softeningEps = e;
}
