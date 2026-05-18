#include "AttractionField.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"

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
	_sources.remove_if([&s](const std::weak_ptr<AttractiveBehaviour>& w) {
		return w.lock() == s;
	});
}

sf::Vector2f AttractionField::EvaluateForce(
    sf::Vector2f worldPos, const PhysicsBodyBehaviour::GroupSet& interactionGroups) const {
	return EvaluateForceImpl(worldPos, interactionGroups, nullptr);
}

sf::Vector2f AttractionField::EvaluateForce(const shared_ptr<AttractiveBehaviour>& receiver) const {
	if (!receiver) {
		return {};
	}
	const auto recvNode = receiver->GetNode();
	if (!recvNode) {
		return {};
	}
	const sf::Vector2f posI = Utils::GetWorldPos(recvNode);
	if (Utils::IsNan(posI)) {
		return {};
	}
	const auto recvRb = recvNode->FindBehaviour<PhysicsBodyBehaviour>();
	if (!recvRb) {
		return {};
	}
	return EvaluateForceImpl(posI, recvRb->GetCollisionGroups(), receiver.get());
}

sf::Vector2f AttractionField::EvaluateForceImpl(sf::Vector2f posI,
    const PhysicsBodyBehaviour::GroupSet& interactionGroups, const AttractiveBehaviour* excludeSource) const {
	_sources.remove_if([](const std::weak_ptr<AttractiveBehaviour>& w) {
		return w.expired();
	});

	sf::Vector2f result{};
	for (const auto& w : _sources) {
		const auto other = w.lock();
		if (!other || !other->IsEnabled()) {
			continue;
		}
		if (excludeSource && other.get() == excludeSource) {
			continue;
		}
		if (other->GetAttraction() == 0.f) {
			continue;
		}
		const auto otherNode = other->GetNode();
		if (!otherNode) {
			continue;
		}
		const auto otherRb = otherNode->FindBehaviour<PhysicsBodyBehaviour>();
		if (!otherRb) {
			continue;
		}
		if (!(interactionGroups & otherRb->GetCollisionGroups()).any()) {
			continue;
		}
		auto otherPos = Utils::GetWorldPos(otherNode);
		if (Utils::IsNan(otherPos)) {
			continue;
		}

		sf::Vector2f d = otherPos - posI;
		const float d2 = d.x * d.x + d.y * d.y;
		const float e2 = _softeningEps * _softeningEps;
		const float r2 = d2 + e2;
		if (r2 < 1e-8f) {
			continue;
		}
		const float invR = 1.f / std::sqrt(r2);
		const float n = other->GetFalloffExponent();
		const float invDistPow = std::pow(invR, n + 1.f);

		const float t = std::abs(other->GetAttraction()) / 100.f;
		const float mag = std::pow(t, 1.2f);
		const float dir = (other->GetAttraction() > 0.f) ? 1.f : -1.f;
		const float scalar = _globalStrengthScale * mag * dir * invDistPow;

		float sourceWeight = 1.f;
		if (_useMassCoupling) {
			if (const auto srb = otherNode->FindBehaviour<PhysicsBodyBehaviour>()) {
				sourceWeight = srb->GetMass();
			}
		}

		auto force = scalar * sourceWeight * d;
		assert(!Utils::IsNan(force));
		result += force;
	}
	return result;
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
