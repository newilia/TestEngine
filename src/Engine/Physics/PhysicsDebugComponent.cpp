#include "PhysicsDebugComponent.h"

#include <fmt/format.h>
#include "AbstractBody.h"
#include "Engine/FontManager.h"
#include "Engine/EngineInterface.h"
#include "Engine/VectorArrow.h"

void PhysicsDebugComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!EI()->isDebugEnabled()) {
		return;
	}
	auto body = dynamic_cast<AbstractBody*>(mHolder);
	if (!body) {
		return;
	}
	auto physComp = body->findComponent<PhysicalComponent>();
	if (!physComp) {
		return;
	}

	sf::Text text;
	auto font = EI()->getFontManager()->getDefaultFont();
	text.setFont(*font);
	text.setCharacterSize(15);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	/*for (size_t i = 0; i < body->getPointCount(); ++i) {
		text.setString(fmt::to_string(i));
		text.setPosition(body->getPointGlobal(i) - sf::Vector2f(3, 10));
		target.draw(text, states);
	}*/

	auto pos = body->getPosGlobal();
	text.setPosition(pos);
	text.setString(fmt::format("{}\n{:.1f}, {:.1f}", body->getName(), pos.x, pos.y));
	target.draw(text, states);

	VectorArrow velArrow(body->getPosGlobal(), body->getPosGlobal() + physComp->mVelocity);
	target.draw(velArrow);
}
