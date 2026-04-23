#pragma once

#include "Engine/Behaviour.h"

/// Регистрирует единственный визуал ноды по sf::Shape из ShapeColliderBehaviourBase на той же ноде.
class ShapeVisualBehaviour : public Behaviour
{
public:
	void OnAttached() override;
};
