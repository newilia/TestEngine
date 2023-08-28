#pragma once
#include <SFML/Graphics/Font.hpp>

#include "Singleton.h"


class FontManager : public Singleton<FontManager> {
public:
	FontManager();
	const sf::Font& getDefaultFont() const;

private:
	void initDefaultFont();
	sf::Font mDefaultFont;
};
