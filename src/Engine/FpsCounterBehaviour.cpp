#include "FpsCounterBehaviour.h"

#include "EngineInterface.h"
#include "FpsNodeVisual.h"
#include "SceneNode.h"

#include <SFML/Graphics/Text.hpp>

#include <cassert>
#include <fmt/format.h>

std::shared_ptr<SceneNode> CreateFpsCounterNode() {
	auto node = std::make_shared<SceneNode>();
	node->setName("Fps");

	EngineContext& engine = EngineContext::Instance();
	auto* font = engine.GetFontManager()->getDefaultFont();
	assert(font);

	sf::Vector2f pos = {engine.GetMainWindow()->getSize().x - 50.f, 0.f};
	auto text = std::make_shared<sf::Text>(*font, "", 15);
	text->setFillColor(sf::Color::White);
	text->setPosition(pos);

	node->AddBehaviour(std::make_shared<FpsCounterBehaviour>(std::move(text)));
	return node;
}

FpsCounterBehaviour::FpsCounterBehaviour(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

void FpsCounterBehaviour::OnAttached() {
	if (!_text) {
		return;
	}
	auto visual = std::make_shared<FpsNodeVisual>(_text);
	if (auto node = GetNode()) {
		node->SetVisual(std::move(visual));
	}
}

void FpsCounterBehaviour::OnUpdate(const sf::Time& dt) {
	(void)dt;
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
