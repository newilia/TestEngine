#pragma once
#include "AbstractBody.h"
#include "SFML/Graphics.hpp"

class AbstractShapeBody : public AbstractBody
{
public:
	AbstractShapeBody() {}

	~AbstractShapeBody() override = default;

	sf::Shape* GetBaseShape() const { return _shape; }

	sf::FloatRect GetBbox() const override { return _shape->getGlobalBounds(); }

	size_t GetPointCount() const override { return _shape->getPointCount(); }

	sf::Vector2f GetPointGlobal(std::size_t index) const override {
		return _shape->getTransform().transformPoint(_shape->getPoint(index));
	}

	void Init() override;

	sf::Vector2f GetPosGlobal() const override { return _shape->getPosition(); }

	void SetPosGlobal(sf::Vector2f pos) override { return _shape->setPosition(pos); }

protected:
	sf::Shape* _shape = nullptr;
};
