#pragma once

#include "Engine/Core/MetaClass.h"
#include "Visual.h"

#include <SFML/Graphics/Font.hpp>

#include <memory>
#include <string>

namespace sf {
	class Text;
}

class TextVisual : public Visual
{
	META_CLASS()
	META_PROPERTY_BASE(Visual)

public:
	TextVisual();
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;

	const sf::Font* GetFont() const;
	void Init(const sf::Font& font, const std::string& string = "", int characterSize = 0);

	const sf::Text* GetText() const;
	sf::FloatRect GetLocalBounds() const;

public:
	/// @getter
	std::string GetString() const;
	/// @setter
	void SetString(const std::string& text);
	/// @getter
	sf::Color GetFillColor() const;
	/// @setter
	void SetFillColor(const sf::Color& color);
	/// @getter
	sf::Color GetOutlineColor() const;
	/// @setter
	void SetOutlineColor(const sf::Color& color);
	/// @getter
	int GetCharacterSize() const;
	/// @setter
	void SetCharacterSize(int size);
	/// @getter
	sf::Vector2f GetOrigin() const;
	/// @setter
	void SetOrigin(const sf::Vector2f& point);
	/// @getter
	sf::Vector2f GetPosition() const;
	/// @setter
	void SetPosition(const sf::Vector2f& position);
	/// @getter
	float GetOutlineThickness() const;
	/// @setter
	void SetOutlineThickness(float thickness);

private:
	std::shared_ptr<sf::Text> _text;
};
