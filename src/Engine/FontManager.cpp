#include "FontManager.h"

FontManager::FontManager() {
	initDefaultFont();
}

const sf::Font* FontManager::getDefaultFont() const {
	return &mDefaultFont;
}

void FontManager::initDefaultFont() {
	mDefaultFont = sf::Font{"resources/fonts/calibri.ttf"};
}
