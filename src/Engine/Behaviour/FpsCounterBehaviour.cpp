#include "FpsCounterBehaviour.h"

#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Visual/TextVisual.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <fmt/format.h>

#include <cassert>

std::shared_ptr<SceneNode> CreateFpsCounterNode() {
	auto node = std::make_shared<SceneNode>();
	node->SetName("Fps");

	auto& context = Engine::MainContext::GetInstance();
	auto* font = context.GetFontManager()->GetDefaultFont();
	assert(font);

	sf::Vector2f pos = {context.GetMainWindow()->getSize().x - 220.f, 0.f};
	auto text = std::make_shared<TextVisual>();
	text->Init(*font);
	text->SetFillColor(sf::Color::White);
	node->GetLocalTransform()->SetPosition(pos);
	node->SetVisual(text);

	auto beh = std::make_shared<FpsCounterBehaviour>();
	beh->SetTextVisual(text);
	node->AddBehaviour(beh);
	return node;
}

void FpsCounterBehaviour::OnUpdate(const sf::Time&) {
	if (auto textVisual = _textVisual.lock()) {
		auto fps = Engine::MainContext::GetInstance().GetCurrentFps();
		auto tickRate = Engine::MainContext::GetInstance().GetCurrentTickRate();
		textVisual->SetString(fmt::format("FPS = {:.0f} | Tick = {:.0f}", fps, tickRate));
	}
}

void FpsCounterBehaviour::SetTextVisual(std::shared_ptr<TextVisual> textVisual) {
	_textVisual = std::move(textVisual);
}
