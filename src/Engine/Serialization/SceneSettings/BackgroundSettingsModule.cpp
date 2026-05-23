#include "Engine/Background/GameBackgroundContext.h"
#include "Engine/Background/PlainColorGameBackground.h"
#include "Engine/Background/TiledGameBackground.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Serialization/PropertyTreeSerializer.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

#include <string_view>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Background";
		constexpr const char kTypeAttr[] = "type";
		constexpr const char kPropertiesElement[] = "Properties";
		constexpr const char kTypeNone[] = "None";
		constexpr const char kTypePlainColor[] = "PlainColor";
		constexpr const char kTypeTiled[] = "Tiled";

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
				else if (dynamic_cast<TiledGameBackground*>(bg) != nullptr) {
					node.append_attribute(kTypeAttr).set_value(kTypeTiled);
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

				if (std::string_view{type} == kTypeTiled) {
					ctx->SetTiledBackground({}, 1.f, 0.f, 256.f);
					if (auto* tiled = dynamic_cast<TiledGameBackground*>(ctx->GetBackground())) {
						if (const pugi::xml_node propertiesNode = node.child(kPropertiesElement)) {
							result.Merge(PropertyTreeSerializer::LoadProvider(*tiled, propertiesNode));
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
