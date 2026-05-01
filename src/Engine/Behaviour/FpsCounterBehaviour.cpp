#include "FpsCounterBehaviour.h"

#include "Engine/App/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/TextVisual.h"
#include "FpsCounterBehaviour.generated.hpp"

#include <SFML/Graphics/Text.hpp>

#include <fmt/format.h>

#include <cassert>

std::shared_ptr<SceneNode> CreateFpsCounterNode() {
	auto node = std::make_shared<SceneNode>();
	node->SetName("Fps");

	auto& context = Engine::MainContext::GetInstance();
	auto* font = context.GetFontManager()->GetDefaultFont();
	assert(font);

	sf::Vector2f pos = {context.GetMainWindow()->getSize().x - 220.f, 0.f};
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

void FpsCounterBehaviour::OnUpdate(const sf::Time&) {
	if (_text) {
		auto fps = Engine::MainContext::GetInstance().GetCurrentFps();
		auto tickRate = Engine::MainContext::GetInstance().GetCurrentTickRate();
		_text->setString(fmt::format("FPS = {:.0f} | Tick = {:.0f}", fps, tickRate));
		_text->setFillColor(_textColor);
	}
}
