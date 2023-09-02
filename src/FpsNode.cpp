#include "FpsNode.h"

#include "GlobalInterface.h"
#include "fmt/format.h"

FpsNode::FpsNode() {
    auto gi = GlobalInterface::getInstance();
    auto font = gi->getFontManager()->getDefaultFont();
    mText.setFillColor(sf::Color::White);
    sf::Vector2f pos = { gi->getMainWindow()->getSize().x - 50.f, 0.f };
    mText.setPosition(sf::Vector2f(pos));
    mText.setCharacterSize(15);
    mText.setFont(*font);
}

void FpsNode::update(const sf::Time& dt) {
    float newFps = 1000000.f / dt.asMicroseconds();
    if (mFps == 0.f) {
        mFps = newFps;
    }
    else {
        constexpr float smoothFactor = 0.95f;
        mFps = mFps * smoothFactor + newFps * (1.f - smoothFactor);
    }
    mText.setString(fmt::format("{:.0f}", mFps));
}

void FpsNode::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(mText, states);
}
