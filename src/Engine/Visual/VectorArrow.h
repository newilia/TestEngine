#pragma once

#include "SFML/Graphics.hpp"

class VectorArrow : public sf::Drawable
{
public:
	VectorArrow() = default;
	VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color = sf::Color::White);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void SetStartPos(const sf::Vector2f& start);
	void SetEndPos(const sf::Vector2f& end);
	void SetColor(const sf::Color& color);
	void SetArrowHeadAngle(float angle);
	void SetArrowHeadSize(float size);

	sf::Vector2f GetStartPos() const;
	sf::Vector2f GetEndPos() const;
	sf::Color GetColor() const;
	float GetArrowHeadAngle() const;
	float GetArrowHeadSize() const;

private:
	sf::Vector2f _start;
	sf::Vector2f _end;
	sf::Color _color = sf::Color::White;
	float _arrowHeadAngle = 0.5f;
	float _arrowHeadSize = 15;
};
