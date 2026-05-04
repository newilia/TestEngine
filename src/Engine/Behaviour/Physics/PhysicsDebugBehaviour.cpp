#include "PhysicsDebugBehaviour.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/TextVisual.h"
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

	const auto pos = node->GetPosGlobal();
	sf::RenderStates dbg = states;
	dbg.transform = sf::Transform{};

	auto* font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont();
	if (font) {
		TextVisual label;
		label.Init(*font);
		label.SetCharacterSize(15);
		label.SetFillColor(sf::Color::White);
		label.SetOutlineColor(sf::Color::Black);
		label.SetOutlineThickness(1.f);
		label.SetPosition(pos);
		label.SetString(fmt::format("{}\n{:.1f}, {:.1f}", node->GetName(), pos.x, pos.y));
		label.Draw(target, dbg);
	}

	VectorArrow velArrow(pos, pos + rigidBody->GetVelocity());
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
