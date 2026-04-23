#pragma once

#include "AbstractBody.h"
#include "Engine/Behaviour.h"

#include <SFML/Graphics/Shape.hpp>

/// Коллайдер на базе sf::Shape: геометрия для физики и связь с нодой сцены.
class ShapeColliderBehaviourBase : public Behaviour, public AbstractBody
{
public:
	~ShapeColliderBehaviourBase() override;

	void OnAttached() override;

	sf::FloatRect GetBbox() const override;

	size_t GetPointCount() const override;

	sf::Vector2f GetPointGlobal(std::size_t index) const override;

	sf::Vector2f GetPosGlobal() const override;

	void SetPosGlobal(sf::Vector2f pos) override;

	virtual sf::Shape* GetBaseShape() const = 0;

private:
	bool _registered = false;
};
