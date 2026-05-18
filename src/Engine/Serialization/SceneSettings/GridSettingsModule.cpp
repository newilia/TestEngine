#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorSceneGrid.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

#include <algorithm>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Grid";
		constexpr const char kVisibleAttr[] = "visible";
		constexpr const char kSizeAttr[] = "size";
		constexpr const char kBasisAttr[] = "basis";
		constexpr const char kSnapAttr[] = "snap";

		class GridSettingsModule final : public ISceneSettingsModule
		{
		public:
			std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const EditorSceneGrid& grid = Editor::GetInstance().GetEditorSceneGrid();
				pugi::xml_node node = settingsParent.append_child(kElementName);
				node.append_attribute(kVisibleAttr).set_value(grid.IsVisible());
				node.append_attribute(kSizeAttr).set_value(grid.GetSize());
				node.append_attribute(kBasisAttr).set_value(grid.GetBasis());
				node.append_attribute(kSnapAttr).set_value(grid.IsSnapEnabled());
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& /*result*/) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				EditorSceneGrid& grid = Editor::GetInstance().GetEditorSceneGrid();
				if (const pugi::xml_attribute attr = node.attribute(kVisibleAttr)) {
					grid.VisibleMutable() = attr.as_bool();
				}
				if (const pugi::xml_attribute attr = node.attribute(kSizeAttr)) {
					grid.SizeMutable() =
					    std::clamp(attr.as_int(), EditorSceneGrid::kMinSize, EditorSceneGrid::kMaxSize);
				}
				if (const pugi::xml_attribute attr = node.attribute(kBasisAttr)) {
					grid.BasisMutable() =
					    std::clamp(attr.as_int(), EditorSceneGrid::kMinBasis, EditorSceneGrid::kMaxBasis);
				}
				if (const pugi::xml_attribute attr = node.attribute(kSnapAttr)) {
					grid.SnapEnabledMutable() = attr.as_bool();
				}
			}

			void ApplyDefaults() const override {
				Editor::GetInstance().GetEditorSceneGrid().ResetToDefaults();
			}
		};

		const GridSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetGridSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
