#pragma once
#include "AbstractBody.h"
#include "SFML/Graphics.hpp"

class AbstractShapeBody : public AbstractBody {
public:
	AbstractShapeBody() {}
	~AbstractShapeBody() override = default;

	sf::Shape* getBaseShape() const { return mShape; }
	sf::FloatRect getBbox() const override { return mShape->getGlobalBounds(); }
	size_t getPointCount() const override { return mShape->getPointCount(); }
	sf::Vector2f getPointGlobal(std::size_t index) const override {
		return mShape->getTransform().transformPoint(mShape->getPoint(index));
	}
	//void update(const sf::Time& dt) override;
	void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;
	sf::Vector2f getPosGlobal() const override { return mShape->getPosition(); }
	void setPosGlobal(sf::Vector2f pos) override { return mShape->setPosition(pos); }

protected:
	sf::Shape* mShape = nullptr;
};

inline void AbstractShapeBody::drawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(*mShape, states);
}