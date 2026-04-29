#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/SceneNode.h"

class InverseSquareFieldSettingsBehaviour : public Behaviour
{
	META_CLASS()
public:
	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;

	/// @property(name="Field strength scale", minValue=0.0, maxValue=1000000.0, step=1, dragSpeed=2, tooltip="Global
	/// scalar for inverse-square central field in world space.")
	float _strengthScale = 50.f;
	/// @property(name="Mass couples field", tooltip="If true, source mass scales contribution; if false, charge only
	/// (no mass).")
	bool _useMassCoupling = true;
};

std::shared_ptr<SceneNode> CreateInverseSquareFieldTuningNode();
