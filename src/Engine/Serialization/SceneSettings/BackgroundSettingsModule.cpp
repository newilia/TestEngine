#include "Engine/Background/GameBackgroundContext.h"
#include "Engine/Background/ParallaxTextureGameBackground.h"
#include "Engine/Background/PlainColorGameBackground.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/PropertyTreeSerializer.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

#include <filesystem>
#include <string_view>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Background";
		constexpr const char kTypeAttr[] = "type";
		constexpr const char kPropertiesElement[] = "Properties";
		constexpr const char kTypeNone[] = "None";
		constexpr const char kTypePlainColor[] = "PlainColor";
		constexpr const char kTypeParallaxTexture[] = "ParallaxTexture";
		constexpr const char kTexturePathAttr[] = "texturePath";
		constexpr const char kOpacityAttr[] = "opacity";
		constexpr const char kScaleWithCameraAttr[] = "scaleWithCamera";
		constexpr const char kMoveWithCameraAttr[] = "moveWithCamera";
		constexpr const char kDefaultScaleAttr[] = "defaultScale";

		class BackgroundSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& result) const override {
				const auto ctx = MainContext::GetInstance().GetGameBackgroundContext();
				if (!ctx) {
					return;
				}
				IGameBackground* bg = ctx->GetBackground();
				if (!bg) {
					return;
				}

				pugi::xml_node node = settingsParent.append_child(kElementName);
				if (dynamic_cast<PlainColorGameBackground*>(bg) != nullptr) {
					node.append_attribute(kTypeAttr).set_value(kTypePlainColor);
				}
				else if (dynamic_cast<ParallaxTextureGameBackground*>(bg) != nullptr) {
					node.append_attribute(kTypeAttr).set_value(kTypeParallaxTexture);
					if (const auto* parallax = dynamic_cast<ParallaxTextureGameBackground*>(bg)) {
						node.append_attribute(kTexturePathAttr).set_value(parallax->GetTexturePath().c_str());
					}
				}
				else {
					return;
				}

				pugi::xml_node propertiesNode = node.append_child(kPropertiesElement);
				result.Merge(
				    PropertyTreeSerializer::SaveProvider(*static_cast<IPropertiesProvider*>(bg), propertiesNode));
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& result) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				const auto ctx = MainContext::GetInstance().GetGameBackgroundContext();
				if (!ctx) {
					return;
				}

				const char* type = node.attribute(kTypeAttr).as_string();
				if (!type || !*type || std::string_view{type} == kTypeNone) {
					ctx->ClearBackground();
					return;
				}

				if (std::string_view{type} == kTypePlainColor) {
					ctx->SetPlainColorBackground(sf::Color(32, 32, 48));
					if (auto* plain = dynamic_cast<PlainColorGameBackground*>(ctx->GetBackground())) {
						if (const pugi::xml_node propertiesNode = node.child(kPropertiesElement)) {
							result.Merge(PropertyTreeSerializer::LoadProvider(*plain, propertiesNode));
						}
					}
					return;
				}

				if (std::string_view{type} == kTypeParallaxTexture) {
					const char* texturePath = node.attribute(kTexturePathAttr).as_string();
					float opacity = node.attribute(kOpacityAttr).as_float(1.f);
					float scaleWithCamera = node.attribute(kScaleWithCameraAttr).as_float(0.f);
					float moveWithCamera = node.attribute(kMoveWithCameraAttr).as_float(0.35f);
					float defaultScale = node.attribute(kDefaultScaleAttr).as_float(256.f);
					ctx->SetParallaxTextureBackground(
					    texturePath ? std::filesystem::path{texturePath} : std::filesystem::path{}, opacity,
					    scaleWithCamera, moveWithCamera, defaultScale);
					if (auto* parallax = dynamic_cast<ParallaxTextureGameBackground*>(ctx->GetBackground())) {
						if (const pugi::xml_node propertiesNode = node.child(kPropertiesElement)) {
							result.Merge(PropertyTreeSerializer::LoadProvider(*parallax, propertiesNode));
						}
					}
				}
			}

			void ApplyDefaults() const override {
				if (const auto ctx = MainContext::GetInstance().GetGameBackgroundContext()) {
					ctx->ClearBackground();
				}
			}
		};

		const BackgroundSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetBackgroundSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
