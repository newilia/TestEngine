#include "FpsNode.h"

#include "EngineInterface.h"
#include "fmt/format.h"

FpsNode::FpsNode() {
    EngineContext& engine = EngineContext::instance();
    auto* font = engine.getFontManager()->getDefaultFont();
	if (!font) {
		assert(font);
		return;
    }
    sf::Vector2f pos = { engine.getMainWindow()->getSize().x - 50.f, 0.f };
    mText = sf::Text(*font, "", 15);
    mText->setFillColor(sf::Color::White);
    mText->setPosition(pos);
}

void FpsNode::update(const sf::Time& dt) {
    auto frameTime = EngineContext::instance().getFrameDt(true);
    float newFps = 1.f / frameTime.asSeconds();
    if (mFps == 0.f) {
        mFps = newFps;
    }
    else {
        constexpr float smoothFactor = 0.98f;
        mFps = mFps * smoothFactor + newFps * (1.f - smoothFactor);
    }
    if (mText) {
        mText->setString(fmt::format("{:.0f}", mFps));
    }
}

void FpsNode::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
    if (mText) {
        target.draw(*mText, states);
    }
}
