#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Simulation";
		constexpr const char kSpeedMultiplierAttr[] = "speedMultiplier";
		constexpr const char kVsyncAttr[] = "vsync";

		class SimulationSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const MainContext& ctx = MainContext::GetInstance();
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kSpeedMultiplierAttr).set_value(ctx.GetSimSpeedMultiplier());
				node.append_attribute(kVsyncAttr).set_value(ctx.IsVerticalSyncEnabled());
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& result) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				MainContext& ctx = MainContext::GetInstance();
				if (const pugi::xml_attribute attr = node.attribute(kSpeedMultiplierAttr)) {
					ctx.SetSimSpeedMultiplier(attr.as_float());
				}
				if (const pugi::xml_attribute attr = node.attribute(kVsyncAttr)) {
					ctx.SetVerticalSyncEnabled(attr.as_bool());
				}
				(void)result;
			}

			void ApplyDefaults() const override {
				MainContext& ctx = MainContext::GetInstance();
				ctx.SetSimPaused(true);
				ctx.SetSimSpeedMultiplier(1.f);
				ctx.SetVerticalSyncEnabled(false);
			}
		};

		const SimulationSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetSimulationSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
