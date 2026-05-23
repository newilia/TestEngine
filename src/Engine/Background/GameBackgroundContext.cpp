#include "Engine/Background/GameBackgroundContext.h"

#include "Engine/Background/IGameBackground.h"
#include "Engine/Background/PlainColorGameBackground.h"
#include "Engine/Background/TiledGameBackground.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

void GameBackgroundContext::Update(const sf::RenderWindow& window, sf::Time dt) {
	if (_background) {
		_background->Update(window, dt);
	}
}

IGameBackground* GameBackgroundContext::GetBackground() const {
	return _background.get();
}

void GameBackgroundContext::ClearBackground() {
	_background.reset();
}

void GameBackgroundContext::SetPlainColorBackground(const sf::Color& color) {
	auto bg = std::make_unique<PlainColorGameBackground>();
	bg->SetFillColor(color);
	_background = std::move(bg);
}

void GameBackgroundContext::SetTiledBackground(
    const std::filesystem::path& texturePath, float opacity, float staticity, float defaultScale) {
	auto bg = std::make_unique<TiledGameBackground>();
	bg->Configure(texturePath.string(), opacity, staticity, defaultScale);
	_background = std::move(bg);
}
