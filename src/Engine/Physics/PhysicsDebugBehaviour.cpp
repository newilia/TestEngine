#include "PhysicsDebugBehaviour.h"

#include "PhysicalBehaviour.h"
#include "Engine/EngineInterface.h"
#include "Engine/FontManager.h"
#include "Engine/SceneNode.h"
#include "Engine/VectorArrow.h"

#include <fmt/format.h>

void PhysicsDebugBehaviour::DrawDebug(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!EngineContext::Instance().IsDebugEnabled()) {
		return;
	}
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto physComp = node->FindEntity<PhysicalBehaviour>();
	if (!physComp) {
		return;
	}

	auto font = EngineContext::Instance().GetFontManager()->getDefaultFont();
	sf::Text text(*font, "", 15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	auto pos = node->GetPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", node->getName(), pos.x, pos.y));
	target.draw(text, states);

	VectorArrow velArrow(node->GetPosGlobal(), node->GetPosGlobal() + physComp->_velocity);
	target.draw(velArrow, states);
}
