#pragma once
#include "SFML/Graphics.hpp"

class VectorArrow : public sf::Drawable
{
public:
	VectorArrow() {}

	VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color = sf::Color::White)
	    : _start(start), _end(end), _color(color) {}

	~VectorArrow() override = default;

	void setStartPos(const sf::Vector2f& start) { _start = start; }

	void setEndPos(const sf::Vector2f& end) { _end = end; }

	void setColor(const sf::Color& color) { _color = color; }

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void enableArrowHead(bool enable) { _arrowHeadEnabled = enable; }

private:
	sf::Vector2f _start;
	sf::Vector2f _end;
	sf::Color _color = sf::Color::White;
	bool _arrowHeadEnabled = true;
	const float _arrowHeadAngle = 0.5f;
	const float _arrowHeadSize = 15;
};