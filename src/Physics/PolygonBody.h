#pragma once
#include "AbstractBody.h"
#include "SFML/Graphics.hpp"

template <typename sfShape>
class PolygonBody : public AbstractBody {
	static_assert(std::is_base_of_v<sf::Shape, sfShape>);

public:
	PolygonBody();
	virtual ~PolygonBody();

	sfShape* getShape() { return reinterpret_cast<sfShape*>(mShape); }
	sf::FloatRect getBbox() const override { return mShape->getGlobalBounds(); }
	int getPointCount() const override { return mShape->getPointCount(); }
	sf::Vector2f getPoint(std::size_t index) const override { return mShape->getPoint(index) + getPhysicalComponent()->mPos; }
	void update(const sf::Time& dt) override;

	void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;

protected:
	sf::Shape* mShape;
};


template <typename sfShape>
PolygonBody<sfShape>::PolygonBody(): mShape(new sfShape()) {}

template <typename sfShape>
PolygonBody<sfShape>::~PolygonBody() { delete mShape; }

template <typename sfShape>
void PolygonBody<sfShape>::update(const sf::Time& dt) {
	auto comp = getPhysicalComponent();
	mShape->setPosition(comp->mPos);
}

template <typename sfShape>
void PolygonBody<sfShape>::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(*mShape, states);
}

//class RectangleBody : public PolygonBody<sf::RectangleShape> {};
//class CircleBody : public PolygonBody<sf::CircleShape> {};
using RectangleBody = PolygonBody<sf::RectangleShape>;
using CircleBody = PolygonBody<sf::CircleShape>;