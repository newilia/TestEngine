#pragma once
#include <SFML/Graphics/Font.hpp>

class FontManager {
public:
	FontManager();
	const sf::Font* getDefaultFont() const;

private:
	void initDefaultFont();

private:
	sf::Font* mDefaultFont = nullptr;
};
