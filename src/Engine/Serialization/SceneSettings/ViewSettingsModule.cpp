#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "View";
		constexpr const char kCenterXAttr[] = "centerX";
		constexpr const char kCenterYAttr[] = "centerY";
		constexpr const char kSizeXAttr[] = "sizeX";
		constexpr const char kSizeYAttr[] = "sizeY";

		class ViewSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const MainContext& ctx = MainContext::GetInstance();
				const std::optional<sf::Vector2f> center = ctx.GetMainCameraCenter();
				const std::optional<sf::Vector2f> viewSize = ctx.GetMainCameraViewSize();
				if (!center || !viewSize) {
					return;
				}
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kCenterXAttr).set_value(center->x);
				node.append_attribute(kCenterYAttr).set_value(center->y);
				node.append_attribute(kSizeXAttr).set_value(viewSize->x);
				node.append_attribute(kSizeYAttr).set_value(viewSize->y);
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& /*result*/) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				const pugi::xml_attribute centerXAttr = node.attribute(kCenterXAttr);
				const pugi::xml_attribute centerYAttr = node.attribute(kCenterYAttr);
				const pugi::xml_attribute sizeXAttr = node.attribute(kSizeXAttr);
				const pugi::xml_attribute sizeYAttr = node.attribute(kSizeYAttr);
				if (!centerXAttr || !centerYAttr || !sizeXAttr || !sizeYAttr) {
					return;
				}
				MainContext::GetInstance().SetMainCameraView(
				    {centerXAttr.as_float(), centerYAttr.as_float()}, {sizeXAttr.as_float(), sizeYAttr.as_float()});
			}

			void ApplyDefaults() const override {
				MainContext::GetInstance().ResetMainCameraViewToDefault();
			}
		};

		const ViewSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetViewSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
