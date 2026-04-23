#include "PhysicsDebugBehaviour.h"

#include "AbstractBody.h"
#include "PhysicalBehaviour.h"
#include "Engine/EngineInterface.h"
#include "Engine/FontManager.h"
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
	auto body = std::dynamic_pointer_cast<AbstractBody>(node);
	if (!body) {
		return;
	}
	auto physComp = body->FindEntity<PhysicalBehaviour>();
	if (!physComp) {
		return;
	}

	auto font = EngineContext::Instance().GetFontManager()->getDefaultFont();
	sf::Text text(*font, "", 15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	auto pos = body->GetPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", body->getName(), pos.x, pos.y));
	target.draw(text, states);

	VectorArrow velArrow(body->GetPosGlobal(), body->GetPosGlobal() + physComp->_velocity);
	target.draw(velArrow, states);
}
