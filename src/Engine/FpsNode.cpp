#include "FpsNode.h"

#include "EngineInterface.h"
#include "fmt/format.h"

FpsNode::FpsNode() {
	EngineContext& engine = EngineContext::Instance();
	auto* font = engine.GetFontManager()->getDefaultFont();
	if (!font) {
		assert(font);
		return;
	}
	sf::Vector2f pos = {engine.GetMainWindow()->getSize().x - 50.f, 0.f};
	_text = sf::Text(*font, "", 15);
	_text->setFillColor(sf::Color::White);
	_text->setPosition(pos);
}

void FpsNode::Update(const sf::Time& dt) {
	auto frameTime = EngineContext::Instance().GetFrameDt(true);
	float newFps = 1.f / frameTime.asSeconds();
	if (_fps == 0.f) {
		_fps = newFps;
	}
	else {
		constexpr float smoothFactor = 0.98f;
		_fps = _fps * smoothFactor + newFps * (1.f - smoothFactor);
	}
	if (_text) {
		_text->setString(fmt::format("{:.0f}", _fps));
	}
}

void FpsNode::DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}
