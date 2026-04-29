#pragma once

#include "Engine/Behaviour/Behaviour.h"

/// Регистрирует единственный визуал ноды по sf::Shape из ShapeColliderBehaviourBase на той же ноде.
class ShapeVisualBehaviour : public Behaviour
{
public:
	void OnInit() override;
	void OnDeinit() override;

private:
	bool _ownsShapeVisual = false;
};
