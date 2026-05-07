#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Shape.hpp"

class VectorArrowShape : public sf::Shape
{
public:
	VectorArrowShape() = default;
	VectorArrowShape(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color = sf::Color::White);

public:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	sf::Vector2f getPoint(std::size_t index) const override;
	std::size_t getPointCount() const override;

	sf::FloatRect GetLocalBounds() const;

	void SetStartPos(const sf::Vector2f& start);
	void SetEndPos(const sf::Vector2f& end);
	void SetArrowHeadAngle(sf::Angle angle);
	void SetArrowHeadSize(float size);

	sf::Vector2f GetStartPos() const;
	sf::Vector2f GetEndPos() const;
	sf::Angle GetArrowHeadAngle() const;
	float GetArrowHeadSize() const;

	// TODO add line thickness

private:
	void UpdateVertices();

	std::array<sf::Vertex, 5> _vertices;

	sf::Vector2f _start;
	sf::Vector2f _end;
	sf::Angle _arrowHeadAngle = sf::degrees(20);
	float _arrowHeadSize = 12;
};
