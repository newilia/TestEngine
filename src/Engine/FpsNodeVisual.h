#pragma once

#include "NodeVisual.h"

#include <memory>

namespace sf {
class Text;
}

class FpsNodeVisual : public NodeVisual
{
public:
	explicit FpsNodeVisual(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	std::shared_ptr<sf::Text> _text;
};
