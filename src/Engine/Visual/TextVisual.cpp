#include "TextVisual.h"

#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "TextVisual.generated.hpp"

#include <SFML/Graphics/Text.hpp>

#include <string>

TextVisual::TextVisual() {}

void TextVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}

bool TextVisual::HitTest(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	const sf::Transform nw = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfVisual(worldPoint, this, nw);
}

const sf::Font* TextVisual::GetFont() const {
	if (_text) {
		return &_text->getFont();
	}
	return nullptr;
}

void TextVisual::Init(const sf::Font& font, const std::string& string, int characterSize) {
	if (_text) {
		_text->setFont(font);
		_text->setString(string);
		_text->setCharacterSize(static_cast<unsigned>(characterSize));
	}
	else {
		_text = std::make_shared<sf::Text>(font, string, static_cast<unsigned>(characterSize));
	}
}

const sf::Text* TextVisual::GetText() const {
	return _text.get();
}

sf::FloatRect TextVisual::GetLocalBounds() const {
	if (_text) {
		return _text->getLocalBounds();
	}
	return {};
}

std::string TextVisual::GetString() const {
	if (!_text) {
		return {};
	}
	return std::string(_text->getString());
}

void TextVisual::SetString(const std::string& text) {
	if (_text) {
		_text->setString(text);
	}
}

sf::Color TextVisual::GetFillColor() const {
	if (_text) {
		return _text->getFillColor();
	}
	return {};
}

void TextVisual::SetFillColor(const sf::Color& color) {
	if (_text) {
		_text->setFillColor(color);
	}
}

sf::Color TextVisual::GetOutlineColor() const {
	if (_text) {
		return _text->getOutlineColor();
	}
	return {};
}

void TextVisual::SetOutlineColor(const sf::Color& color) {
	if (_text) {
		_text->setOutlineColor(color);
	}
}

int TextVisual::GetCharacterSize() const {
	if (_text) {
		return static_cast<int>(_text->getCharacterSize());
	}
	return 0;
}

void TextVisual::SetCharacterSize(int size) {
	if (_text) {
		_text->setCharacterSize(static_cast<unsigned>(size));
	}
}

sf::Vector2f TextVisual::GetOrigin() const {
	if (_text) {
		return _text->getOrigin();
	}
	return {};
}

void TextVisual::SetOrigin(const sf::Vector2f& point) {
	if (_text) {
		_text->setOrigin(point);
	}
}

void TextVisual::SetPosition(const sf::Vector2f& position) {
	if (_text) {
		_text->setPosition(position);
	}
}

sf::Vector2f TextVisual::GetPosition() const {
	if (_text) {
		return _text->getPosition();
	}
	return {};
}

float TextVisual::GetOutlineThickness() const {
	if (_text) {
		return _text->getOutlineThickness();
	}
	return 0.f;
}

void TextVisual::SetOutlineThickness(const float thickness) {
	if (_text) {
		_text->setOutlineThickness(thickness);
	}
}
