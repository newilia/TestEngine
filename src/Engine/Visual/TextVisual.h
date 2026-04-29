#pragma once

#include "Visual.h"

#include <memory>

namespace sf {
	class Text;
}

class TextVisual : public Visual
{
public:
	explicit TextVisual(std::shared_ptr<sf::Text> text);

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	bool HitTest(sf::Vector2f windowPosition) const override;

	const sf::Text* GetText() const;

private:
	std::shared_ptr<sf::Text> _text;
};
