#include "BodyDebugComponent.h"

#include <fmt/format.h>
#include "AbstractBody.h"
#include "FontManager.h"
#include "EngineInterface.h"
#include "VectorArrow.h"

void BodyDebugComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto body = dynamic_cast<AbstractBody*>(mHolder)) {
		if (auto physComp = body->findComponent<PhysicalComponent>()) {
			sf::Text text;
			for (size_t i = 0; i < body->getPointCount(); ++i) {
				text.setString(fmt::to_string(i));
				text.setCharacterSize(15);
				text.setFillColor(sf::Color::White);
				text.setPosition(body->getPointGlobal(i));
				auto font = EI()->getFontManager()->getDefaultFont();
				text.setFont(*font);
				target.draw(text, states);
			}
			text.setFillColor(sf::Color::White);
			text.setPosition(body->getGlobalCenter());
			text.setString(fmt::format("{:.1f}, {:.1f}", physComp->mPos.x, physComp->mPos.y));
			target.draw(text, states);

			VectorArrow velArrow(body->getGlobalCenter(), body->getGlobalCenter() + physComp->mVelocity);
			target.draw(velArrow);
		}
	}
}
