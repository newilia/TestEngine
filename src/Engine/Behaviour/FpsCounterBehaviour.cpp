#include "FpsCounterBehaviour.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/TextVisual.h"
#include "FpsCounterBehaviour_gen.hpp"

#include <SFML/Graphics/Text.hpp>

#include <fmt/format.h>

#include <cassert>

std::shared_ptr<SceneNode> CreateFpsCounterNode() {
	auto node = std::make_shared<SceneNode>();
	node->SetName("Fps");

	EngineContext& engine = EngineContext::GetInstance();
	auto* font = engine.GetFontManager()->GetDefaultFont();
	assert(font);

	sf::Vector2f pos = {engine.GetMainWindow()->getSize().x - 220.f, 0.f};
	auto text = std::make_shared<sf::Text>(*font, "", 15);
	text->setFillColor(sf::Color::White);
	text->setPosition(pos);

	node->AddBehaviour(std::make_shared<FpsCounterBehaviour>(std::move(text)));
	return node;
}

FpsCounterBehaviour::FpsCounterBehaviour(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

void FpsCounterBehaviour::OnInit() {
	if (!_text) {
		return;
	}
	_text->setFillColor(_textColor);
	auto visual = std::make_shared<TextVisual>(_text);
	if (auto node = GetNode()) {
		node->SetVisual(std::move(visual));
		_ownsDisplayVisual = true;
	}
}

void FpsCounterBehaviour::OnDeinit() {
	if (!_ownsDisplayVisual) {
		return;
	}
	if (auto node = GetNode()) {
		node->SetVisual(nullptr);
	}
	_ownsDisplayVisual = false;
}

void FpsCounterBehaviour::OnUpdate(const sf::Time& /*dt*/) {}

void FpsCounterBehaviour::OnPresent(const sf::Time& realFrameDt) {
	// todo get tickHz from EngineContext when available (needs to be set there first)

	if (_text) {
		_text->setString(fmt::format("FPS = {:.0f} | Tick = {:.0f}", _fps, _tickHz));
		_text->setFillColor(_textColor);
	}
}
