#include "FpsNode.h"

#include "EngineInterface.h"
#include "fmt/format.h"

FpsNode::FpsNode() {
    auto ei = EI();
    auto font = ei->getFontManager()->getDefaultFont();
    mText.setFillColor(sf::Color::White);
    sf::Vector2f pos = { ei->getMainWindow()->getSize().x - 50.f, 0.f };
    mText.setPosition(sf::Vector2f(pos));
    mText.setCharacterSize(15);
    mText.setFont(*font);
}

void FpsNode::update(const sf::Time& dt) {
    auto frameTime = EI()->getFrameDt(true);
    float newFps = 1.f / frameTime.asSeconds();
    if (mFps == 0.f) {
        mFps = newFps;
    }
    else {
        constexpr float smoothFactor = 0.98f;
        mFps = mFps * smoothFactor + newFps * (1.f - smoothFactor);
    }
    mText.setString(fmt::format("{:.0f}", mFps));
}

void FpsNode::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(mText, states);
}
