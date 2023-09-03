#include "DebugComponent.h"

#include <fmt/format.h>
#include "AbstractBody.h"
#include "FontManager.h"
#include "EngineInterface.h"

void DebugComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto body = dynamic_cast<AbstractBody*>(mHolder)) {
		if (auto physComp = body->findComponent<PhysicalComponent>()) {
			sf::Text text;
			for (size_t i = 0; i < body->getPointCount(); ++i) {
				text.setString(fmt::to_string(i));
				text.setCharacterSize(15);
				text.setFillColor(sf::Color::White);
				text.setPosition(body->getPoint(i));
				auto font = EI()->getFontManager()->getDefaultFont();
				text.setFont(*font);
				target.draw(text, states);
			}
			text.setFillColor(sf::Color::White);
			text.setPosition(physComp->mPos + sf::Vector2f(15.f, 15.f));
			text.setString(fmt::format("{:.1f}, {:.1f}", physComp->mPos.x, physComp->mPos.y));
			target.draw(text, states);
		}
	}
}
