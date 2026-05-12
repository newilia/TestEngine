#include "Engine/Editor/PhysicsVisualizer.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/VectorArrowShape.h"

#include <SFML/Graphics/RenderWindow.hpp>

namespace {

	constexpr float kMinArrowLengthSq = 1e-1f;
	constexpr float kVelocityInternalScale = 0.1f;
	constexpr float kAccelInternalScale = 0.001f;

} // namespace

namespace Engine {

	void PhysicsVisualizer::Draw(sf::RenderWindow& window) const {
		if (!_isVelocityVisible && !_isForceVisible) {
			return;
		}
		const auto proc = MainContext::GetInstance().GetPhysicsProcessor();
		if (!proc) {
			return;
		}

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
				const sf::Vector2f delta = proc->EvaluateExternalForces(body.get()) * _forceScale * kAccelInternalScale;
				if (delta.lengthSquared() >= kMinArrowLengthSq) {
					VectorArrowShape arrow(origin, origin + delta, _forceColor);
					arrow.draw(window, sf::RenderStates::Default);
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

} // namespace Engine
