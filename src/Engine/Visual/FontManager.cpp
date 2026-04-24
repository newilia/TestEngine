#include "FontManager.h"

#include <iostream>

FontManager::FontManager() {
	InitDefaultFont();
}

void FontManager::InitDefaultFont() {
	try {
		_defaultFont = new sf::Font{"resources/fonts/calibri.ttf"};
	}
	catch (const std::exception& e) {
		std::cerr << e.what();
	}
	catch (...) {
		throw;
	}
}

const sf::Font* FontManager::GetDefaultFont() const {
	return _defaultFont;
}
