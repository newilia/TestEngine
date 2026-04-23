#include "FontManager.h"
#include <iostream>

FontManager::FontManager() {
	initDefaultFont();
}

void FontManager::initDefaultFont() {
	try {
		mDefaultFont = new sf::Font{"resources/fonts/calibri.ttf"};
	} 
	catch (const std::exception& e) {
		std::cerr << e.what();
	} 
	catch (...) {
		throw;
	}
}

const sf::Font* FontManager::getDefaultFont() const {
	return mDefaultFont;
}
