#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"
#include "Engine/Simulation/AttractionField.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Vector2.hpp>

#include <algorithm>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Physics";
		constexpr const char kGravityXAttr[] = "gravityX";
		constexpr const char kGravityYAttr[] = "gravityY";
		constexpr const char kGravityEnabledAttr[] = "gravityEnabled";
		constexpr const char kAirFrictionAttr[] = "airFriction";
		constexpr const char kSubstepsAttr[] = "substeps";
		constexpr const char kAttractionElement[] = "Attraction";
		constexpr const char kAttractionGlobalStrengthScaleAttr[] = "strength";
		constexpr const char kAttractionSofteningEpsAttr[] = "softening";
		constexpr const char kAttractionUseMassCouplingAttr[] = "massCoupling";

		class PhysicsSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const auto proc = MainContext::GetInstance().GetPhysicsProcessor();
				if (!proc) {
					return;
				}
				const sf::Vector2f gravity = proc->GetGravity();
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kGravityXAttr).set_value(gravity.x);
				node.append_attribute(kGravityYAttr).set_value(gravity.y);
				node.append_attribute(kGravityEnabledAttr).set_value(proc->IsGravityEnabled());
				node.append_attribute(kAirFrictionAttr).set_value(proc->GetAirFriction());
				node.append_attribute(kSubstepsAttr).set_value(proc->GetSimulationSubsteps());

				if (const auto field = proc->GetAttractionField()) {
					pugi::xml_node attraction = node.append_child(kAttractionElement);
					attraction.append_attribute(kAttractionGlobalStrengthScaleAttr)
					    .set_value(field->GetGlobalStrengthScale());
					attraction.append_attribute(kAttractionSofteningEpsAttr).set_value(field->GetSofteningEps());
					attraction.append_attribute(kAttractionUseMassCouplingAttr).set_value(field->GetUseMassCoupling());
				}
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& /*result*/) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				const auto proc = MainContext::GetInstance().GetPhysicsProcessor();
				if (!proc) {
					return;
				}
				sf::Vector2f gravity = proc->GetGravity();
				if (const pugi::xml_attribute attr = node.attribute(kGravityXAttr)) {
					gravity.x = attr.as_float();
				}
				if (const pugi::xml_attribute attr = node.attribute(kGravityYAttr)) {
					gravity.y = attr.as_float();
				}
				if (node.attribute(kGravityXAttr) || node.attribute(kGravityYAttr)) {
					proc->SetGravity(gravity);
				}
				if (const pugi::xml_attribute attr = node.attribute(kGravityEnabledAttr)) {
					proc->SetGravityEnabled(attr.as_bool());
				}
				if (const pugi::xml_attribute attr = node.attribute(kAirFrictionAttr)) {
					proc->SetAirFriction(attr.as_float());
				}
				if (const pugi::xml_attribute attr = node.attribute(kSubstepsAttr)) {
					proc->SetSimulationSubsteps(attr.as_int());
				}

				if (const auto field = proc->GetAttractionField()) {
					if (const pugi::xml_node attraction = node.child(kAttractionElement)) {
						if (const pugi::xml_attribute attr = attraction.attribute(kAttractionGlobalStrengthScaleAttr)) {
							field->SetGlobalStrengthScale(attr.as_float());
						}
						if (const pugi::xml_attribute attr = attraction.attribute(kAttractionSofteningEpsAttr)) {
							field->SetSofteningEps(attr.as_float());
						}
						if (const pugi::xml_attribute attr = attraction.attribute(kAttractionUseMassCouplingAttr)) {
							field->SetUseMassCoupling(attr.as_bool());
						}
					}
				}
			}

			void ApplyDefaults() const override {
				const auto proc = MainContext::GetInstance().GetPhysicsProcessor();
				if (!proc) {
					return;
				}
				proc->SetGravity({0.f, 400.f});
				proc->SetGravityEnabled(false);
				proc->SetAirFriction(0.f);
				proc->SetSimulationSubsteps(1);
				if (const auto field = proc->GetAttractionField()) {
					field->SetGlobalStrengthScale(50.f);
					field->SetSofteningEps(4.f);
					field->SetUseMassCoupling(true);
				}
			}
		};

		const PhysicsSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetPhysicsSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
