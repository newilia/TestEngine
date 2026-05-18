#include "Engine/Render/SceneLighting.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

#include <string_view>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Lighting";
		constexpr const char kEnabledAttr[] = "enabled";
		constexpr const char kDistanceScaleAttr[] = "distanceScale";
		constexpr const char kIntensityScaleAttr[] = "intensityScale";
		constexpr const char kBlendModeAttr[] = "blendMode";

		const char* BlendModeToString(LightingBlendMode mode) {
			switch (mode) {
			case LightingBlendMode::Additive:
				return "Additive";
			case LightingBlendMode::Screen:
				return "Screen";
			}
			return "Screen";
		}

		bool TryParseBlendMode(const char* text, LightingBlendMode& out) {
			if (!text || !*text) {
				return false;
			}
			if (std::string_view{text} == "Additive") {
				out = LightingBlendMode::Additive;
				return true;
			}
			if (std::string_view{text} == "Screen") {
				out = LightingBlendMode::Screen;
				return true;
			}
			return false;
		}

		class LightingSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const SceneLighting& lighting = SceneLighting::GetInstance();
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kEnabledAttr).set_value(lighting.IsEnabled());
				node.append_attribute(kDistanceScaleAttr).set_value(lighting.GetDistanceRangeScale());
				node.append_attribute(kIntensityScaleAttr).set_value(lighting.GetGlobalIntensityScale());
				node.append_attribute(kBlendModeAttr).set_value(BlendModeToString(lighting.GetBlendMode()));
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& /*result*/) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				SceneLighting& lighting = SceneLighting::GetInstance();
				if (const pugi::xml_attribute attr = node.attribute(kEnabledAttr)) {
					lighting.SetEnabled(attr.as_bool());
				}
				if (const pugi::xml_attribute attr = node.attribute(kDistanceScaleAttr)) {
					lighting.SetDistanceRangeScale(attr.as_float());
				}
				if (const pugi::xml_attribute attr = node.attribute(kIntensityScaleAttr)) {
					lighting.SetGlobalIntensityScale(attr.as_float());
				}
				if (const pugi::xml_attribute attr = node.attribute(kBlendModeAttr)) {
					LightingBlendMode mode{};
					if (TryParseBlendMode(attr.as_string(), mode)) {
						lighting.SetBlendMode(mode);
					}
				}
			}

			void ApplyDefaults() const override {
				SceneLighting& lighting = SceneLighting::GetInstance();
				lighting.SetEnabled(true);
				lighting.SetDistanceRangeScale(1.f);
				lighting.SetGlobalIntensityScale(1.f);
				lighting.SetBlendMode(LightingBlendMode::Screen);
			}
		};

		const LightingSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetLightingSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
