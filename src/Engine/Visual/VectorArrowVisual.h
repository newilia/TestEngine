#pragma once

#include "Engine/Visual/Visual.h"
#include "SFML/Graphics.hpp"

/// Линия с опциональным наконечником; риуется как `sf::Drawable`.
class VectorArrow : public sf::Drawable
{
public:
	VectorArrow();

	VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color = sf::Color::White);

	~VectorArrow() override;

	void SetStartPos(const sf::Vector2f& start);
	void SetEndPos(const sf::Vector2f& end);
	void SetColor(const sf::Color& color);
	void EnableArrowHead(bool enable);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Vector2f _start;
	sf::Vector2f _end;
	sf::Color _color = sf::Color::White;
	bool _arrowHeadEnabled = true;
	const float _arrowHeadAngle = 0.5f;
	const float _arrowHeadSize = 15;
};

class VectorArrowVisual : public Visual
{
public:
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	void SetStartPos(const sf::Vector2f& start);
	void SetEndPos(const sf::Vector2f& end);
	void SetColor(const sf::Color& color);
	void SetVisible(bool visible);

private:
	VectorArrow _arrow;
	bool _visible = false;
};
