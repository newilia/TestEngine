#include "PhysicsDebugComponent.h"

#include "AbstractBody.h"
#include "Engine/EngineInterface.h"
#include "Engine/FontManager.h"
#include "Engine/VectorArrow.h"

#include <fmt/format.h>

void PhysicsDebugComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!EngineContext::Instance().IsDebugEnabled()) {
		return;
	}
	auto body = dynamic_cast<AbstractBody*>(_holder);
	if (!body) {
		return;
	}
	auto physComp = body->FindComponent<PhysicalComponent>();
	if (!physComp) {
		return;
	}

	auto font = EngineContext::Instance().GetFontManager()->getDefaultFont();
	sf::Text text(*font, "", 15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	/*for (size_t i = 0; i < body->GetPointCount(); ++i) {
	    text.setString(fmt::to_string(i));
	    text.setPosition(body->GetPointGlobal(i) - sf::Vector2f(3, 10));
	    target.draw(text, states);
	}*/

	auto pos = body->GetPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", body->getName(), pos.x, pos.y));
	target.draw(text, states);

	VectorArrow velArrow(body->GetPosGlobal(), body->GetPosGlobal() + physComp->_velocity);
	target.draw(velArrow);
}
