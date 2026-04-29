#include "PhysicsDebugBehaviour.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/App/FontManager.h"
#include "Engine/Behaviour/Physics/InverseSquareFieldSourceBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsHandler.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <fmt/format.h>

#include <cmath>

void PhysicsDebugBehaviour::DebugDraw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!EngineContext::Instance().IsDebugEnabled()) {
		return;
	}
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto rigidBody = node->FindBehaviour<RigidBodyBehaviour>();
	if (!rigidBody) {
		return;
	}

	auto font = EngineContext::Instance().GetFontManager()->GetDefaultFont();
	sf::Text text(*font, "", 15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	auto pos = node->GetPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", node->GetName(), pos.x, pos.y));
	target.draw(text, states);

	VectorArrow velArrow(node->GetPosGlobal(), node->GetPosGlobal() + rigidBody->_velocity);
	target.draw(velArrow, states);

	if (auto fieldSrc = node->FindBehaviour<InverseSquareFieldSourceBehaviour>()) {
		if (fieldSrc->_isEnabled && !rigidBody->IsImmovable() && std::isfinite(rigidBody->_mass) &&
		    rigidBody->_mass > 0.f) {
			auto ph = EngineContext::Instance().GetPhysicsHandler();
			auto field = ph ? ph->GetIsotropicInverseSquareField() : nullptr;
			if (field) {
				const sf::Vector2f a = field->EvaluateAcceleration(fieldSrc);
				const sf::Vector2f force = a * rigidBody->_mass;
				const float s = EngineContext::Instance().GetFieldForceDebugArrowScale();
				const float vx = force.x * s;
				const float vy = force.y * s;
				const float visualLen2 = vx * vx + vy * vy;
				if (visualLen2 > 1e-10f) {
					const sf::Vector2f tip = pos + sf::Vector2f{vx, vy};
					VectorArrow forceArrow(pos, tip, sf::Color(255, 80, 220));
					target.draw(forceArrow, states);
				}
			}
		}
	}
}
