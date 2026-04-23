#pragma once
#include "AbstractBody.h"
#include "SFML/Graphics.hpp"

class AbstractShapeBody : public AbstractBody
{ // todo: combine with Abstract body
public:
	AbstractShapeBody() {}

	~AbstractShapeBody() override = default;

	sf::Shape* GetBaseShape() const { return _shape; }

	sf::FloatRect getBbox() const override { return _shape->getGlobalBounds(); }

	size_t getPointCount() const override { return _shape->getPointCount(); }

	sf::Vector2f getPointGlobal(std::size_t index) const override {
		return _shape->getTransform().transformPoint(_shape->getPoint(index));
	}

	// void update(const sf::Time& dt) override;
	void DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::Vector2f getPosGlobal() const override { return _shape->getPosition(); }

	void setPosGlobal(sf::Vector2f pos) override { return _shape->setPosition(pos); }

protected:
	sf::Shape* _shape = nullptr;
};

inline void AbstractShapeBody::DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(*_shape, states);
}
