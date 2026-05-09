#include "Engine/Background/GameBackgroundContext.h"

#include "Engine/Background/IGameBackground.h"
#include "Engine/Background/ParallaxTextureGameBackground.h"
#include "Engine/Background/PlainColorGameBackground.h"

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

void GameBackgroundContext::SetParallaxTextureBackground(const std::filesystem::path& texturePath, float opacity,
    float scaleWithCamera, float moveWithCamera, float defaultScale) {
	auto bg = std::make_unique<ParallaxTextureGameBackground>();
	bg->Configure(texturePath.string(), opacity, scaleWithCamera, moveWithCamera, defaultScale);
	_background = std::move(bg);
}
