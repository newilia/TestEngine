#pragma once
#include "SFML/Graphics.hpp"

class VectorArrow : public sf::Drawable {
public:
	VectorArrow() {}
	VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color = sf::Color::White) : mStart(start), mEnd(end), mColor(color) {}
	~VectorArrow() override = default;

	void setStartPos(const sf::Vector2f& start) { mStart = start; }
	void setEndPos(const sf::Vector2f& end) { mEnd = end; }
	void setColor(const sf::Color& color) { mColor = color; }
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void enableArrowHead(bool enable) { mArrowHeadEnabled = enable; }
private:
	sf::Vector2f mStart;
	sf::Vector2f mEnd;
	sf::Color mColor = sf::Color::White;
	bool mArrowHeadEnabled = true;
	const float mArrowHeadAngle = 0.5f;
	const float mArrowHeadSize = 15;
};