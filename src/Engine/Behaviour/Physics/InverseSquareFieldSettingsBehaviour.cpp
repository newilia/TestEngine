#include "InverseSquareFieldSettingsBehaviour.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Simulation/IsotropicInverseSquareField.h"
#include "Engine/Simulation/PhysicsHandler.h"
#include "InverseSquareFieldSettingsBehaviour_gen.hpp"

#include <memory>

std::shared_ptr<SceneNode> CreateInverseSquareFieldTuningNode() {
	auto node = std::make_shared<SceneNode>();
	node->SetName("Field tuning");
	node->AddBehaviour(std::make_shared<InverseSquareFieldSettingsBehaviour>());
	return node;
}

void InverseSquareFieldSettingsBehaviour::OnInit() {
	if (const auto ph = EngineContext::GetInstance().GetPhysicsHandler()) {
		if (const auto field = ph->GetIsotropicInverseSquareField()) {
			_strengthScale = field->GetGlobalStrengthScale();
			_useMassCoupling = field->GetUseMassCoupling();
		}
	}
}

void InverseSquareFieldSettingsBehaviour::OnUpdate(const sf::Time& dt) {
	(void)dt;
	if (const auto ph = EngineContext::GetInstance().GetPhysicsHandler()) {
		if (auto field = ph->GetIsotropicInverseSquareField()) {
			field->SetGlobalStrengthScale(_strengthScale);
			field->SetUseMassCoupling(_useMassCoupling);
		}
	}
}
