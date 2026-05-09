#include "Engine/Background/PlainColorGameBackground.h"

#include "PlainColorGameBackground.generated.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

void PlainColorGameBackground::Update(const sf::RenderWindow& /*window*/, sf::Time /*dt*/) {}

void PlainColorGameBackground::SetFillColor(const sf::Color& color) {
	_fillColor = color;
}

const sf::Color& PlainColorGameBackground::GetFillColor() const {
	return _fillColor;
}

void PlainColorGameBackground::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	const sf::View view = target.getView();
	const sf::Vector2f size = view.getSize();
	const sf::Vector2f center = view.getCenter();

	sf::RectangleShape rect(size);
	rect.setOrigin({size.x * 0.5f, size.y * 0.5f});
	rect.setPosition(center);
	rect.setFillColor(_fillColor);
	target.draw(rect, states);
}
