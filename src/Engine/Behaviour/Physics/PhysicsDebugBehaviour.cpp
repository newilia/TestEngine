#include "PhysicsDebugBehaviour.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/VectorArrowVisual.h"

#include <fmt/format.h>

#include <cmath>

void PhysicsDebugBehaviour::DebugDraw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		return;
	}
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto rigidBody = node->FindBehaviour<PhysicsBodyBehaviour>();
	if (!rigidBody) {
		return;
	}

	auto font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont();
	sf::Text text(*font, "", 15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	auto pos = node->GetPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", node->GetName(), pos.x, pos.y));
	sf::RenderStates dbg = states;
	dbg.transform = sf::Transform{};
	target.draw(text, dbg);

	VectorArrow velArrow(node->GetPosGlobal(), node->GetPosGlobal() + rigidBody->GetVelocity());
	target.draw(velArrow, dbg);

	if (auto fieldSrc = node->FindBehaviour<AttractiveBehaviour>()) {
		if (fieldSrc->IsEnabled() && !rigidBody->IsImmovable() && std::isfinite(rigidBody->GetMass()) &&
		    rigidBody->GetMass() > 0.f) {
			auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor();
			auto field = ph ? ph->GetAttractionField() : nullptr;
			if (field) {
				const sf::Vector2f a = field->EvaluateAcceleration(fieldSrc);
				const sf::Vector2f force = a * rigidBody->GetMass();
				const float s = Engine::MainContext::GetInstance().GetFieldForceDebugArrowScale();
				const float vx = force.x * s;
				const float vy = force.y * s;
				const float visualLen2 = vx * vx + vy * vy;
				if (visualLen2 > 1e-10f) {
					const sf::Vector2f tip = pos + sf::Vector2f{vx, vy};
					VectorArrow forceArrow(pos, tip, sf::Color(255, 80, 220));
					target.draw(forceArrow, dbg);
				}
			}
		}
	}
}
