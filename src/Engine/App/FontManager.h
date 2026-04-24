#pragma once
#include <SFML/Graphics/Font.hpp>

class FontManager
{
public:
	FontManager();
	const sf::Font* GetDefaultFont() const;

private:
	void InitDefaultFont();

private:
	sf::Font* _defaultFont = nullptr;
};
