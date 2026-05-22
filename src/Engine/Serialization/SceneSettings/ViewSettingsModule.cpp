#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "View";
		constexpr const char kCenterXAttr[] = "centerX";
		constexpr const char kCenterYAttr[] = "centerY";
		constexpr const char kScaleAttr[] = "scale";
		constexpr const char kLegacySizeXAttr[] = "sizeX";
		constexpr const char kLegacySizeYAttr[] = "sizeY";

		[[nodiscard]] std::optional<float> ReadViewScaleFromNode(const pugi::xml_node& node) {
			const pugi::xml_attribute scaleAttr = node.attribute(kScaleAttr);
			if (scaleAttr) {
				return scaleAttr.as_float();
			}
			const pugi::xml_attribute sizeXAttr = node.attribute(kLegacySizeXAttr);
			const pugi::xml_attribute sizeYAttr = node.attribute(kLegacySizeYAttr);
			if (!sizeXAttr || !sizeYAttr) {
				return std::nullopt;
			}
			const sf::RenderWindow* window = MainContext::GetInstance().GetMainWindow();
			if (!window) {
				return std::nullopt;
			}
			const sf::Vector2u pixelSize = window->getSize();
			if (pixelSize.x == 0u || pixelSize.y == 0u) {
				return std::nullopt;
			}
			const float w = static_cast<float>(pixelSize.x);
			const float h = static_cast<float>(pixelSize.y);
			return (sizeXAttr.as_float() / w + sizeYAttr.as_float() / h) * 0.5f;
		}

		class ViewSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const MainContext& ctx = MainContext::GetInstance();
				const std::optional<sf::Vector2f> center = ctx.GetMainCameraCenter();
				const std::optional<float> scale = ctx.GetMainCameraViewScale();
				if (!center || !scale) {
					return;
				}
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kCenterXAttr).set_value(center->x);
				node.append_attribute(kCenterYAttr).set_value(center->y);
				node.append_attribute(kScaleAttr).set_value(*scale);
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& /*result*/) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				const pugi::xml_attribute centerXAttr = node.attribute(kCenterXAttr);
				const pugi::xml_attribute centerYAttr = node.attribute(kCenterYAttr);
				if (!centerXAttr || !centerYAttr) {
					return;
				}
				const std::optional<float> scale = ReadViewScaleFromNode(node);
				if (!scale) {
					return;
				}
				MainContext::GetInstance().SetMainCameraView({centerXAttr.as_float(), centerYAttr.as_float()}, *scale);
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
