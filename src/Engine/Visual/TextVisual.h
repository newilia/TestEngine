#pragma once

#include "NodeVisual.h"

#include <memory>

namespace sf {
class Text;
}

class TextVisual : public NodeVisual
{
public:
	explicit TextVisual(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	bool HitTest(sf::Vector2f windowPosition) const override;

	const sf::Text* GetText() const { return _text.get(); }

private:
	std::shared_ptr<sf::Text> _text;
};
