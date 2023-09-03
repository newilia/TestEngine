#pragma once
#include "AbstractBody.h"
#include "SFML/Graphics.hpp"

class AbstractShapeBody : public AbstractBody {
public:
	AbstractShapeBody() {}
	~AbstractShapeBody() override = default;

	sf::Shape* getBaseShape() { return mShape; }
	sf::Shape const* getBaseShape() const { return mShape; }
	sf::FloatRect getBbox() const override { return mShape->getGlobalBounds(); }
	size_t getPointCount() const override { return mShape->getPointCount(); }
	sf::Vector2f getPointGlobal(std::size_t index) const override {
		return mShape->getPoint(index) + mShape->getPosition();
	}
	void update(const sf::Time& dt) override;
	void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;
	sf::Vector2f getGlobalCenter() const override {
		auto bbox = mShape->getGlobalBounds();
		return sf::Vector2f(bbox.left + bbox.width * 0.5f, bbox.top + bbox.height * 0.5f);
	}

protected:
	virtual void initShape() = 0;
	sf::Shape* mShape = nullptr;
};

inline void AbstractShapeBody::update(const sf::Time& dt) {
	auto comp = getPhysicalComponent();
	mShape->setPosition(comp->mPos);
}

inline void AbstractShapeBody::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(*mShape, states);
}


template <typename TShape>
class ShapeBody : public AbstractShapeBody {
	static_assert(std::is_base_of_v<sf::Shape, TShape>);
public:
	void initShape() override { mShape = new TShape; }
	TShape* getShape() const { return dynamic_cast<TShape*>(mShape); }
	ShapeBody();
	~ShapeBody() override;
};

template <typename sfShape>
ShapeBody<sfShape>::ShapeBody() { ShapeBody::initShape(); }

template <typename sfShape>
ShapeBody<sfShape>::~ShapeBody() { delete mShape; }

using RectangleBody = ShapeBody<sf::RectangleShape>;
using CircleBody = ShapeBody<sf::CircleShape>;