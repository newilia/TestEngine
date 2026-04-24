#include "PhysicsDebugBehaviour.h"

#include "Engine/Physics/RigidBodyBehaviour.h"
#include "Engine/App/EngineInterface.h"
#include "Engine/Visual/FontManager.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/VectorArrow.h"

#include <fmt/format.h>

void PhysicsDebugBehaviour::DrawDebug(sf::RenderTarget& target, sf::RenderStates states) const {
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
}
