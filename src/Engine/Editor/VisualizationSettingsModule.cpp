#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/PhysicsVisualizer.h"
#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

#include <SFML/Graphics/Color.hpp>

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

namespace Engine::Serialization {
	namespace {

		constexpr const char kElementName[] = "Visualization";
		constexpr const char kVelocityElement[] = "Velocity";
		constexpr const char kForceElement[] = "Force";
		constexpr const char kAttractionFieldElement[] = "AttractionField";
		constexpr const char kVisibleAttr[] = "visible";
		constexpr const char kScaleAttr[] = "scale";
		constexpr const char kColorAttr[] = "color";
		constexpr const char kGridLayoutAttr[] = "gridLayout";
		constexpr const char kSpacingPxAttr[] = "spacingPx";
		constexpr const char kArrowScaleAttr[] = "arrowScale";
		constexpr const char kArrowLengthSoftCapAttr[] = "arrowLengthSoftCap";
		constexpr const char kArrowLengthCompressAttr[] = "arrowLengthCompress";
		constexpr const char kPaletteSpanAttr[] = "paletteSpan";
		constexpr const char kPaletteWeakAttr[] = "paletteWeak";
		constexpr const char kPaletteMidAttr[] = "paletteMid";
		constexpr const char kPaletteStrongAttr[] = "paletteStrong";
		constexpr const char kProbeGroupsAttr[] = "probeGroups";

		std::string FormatColor(sf::Color color) {
			return std::to_string(static_cast<unsigned>(color.r)) + ',' +
			       std::to_string(static_cast<unsigned>(color.g)) + ',' +
			       std::to_string(static_cast<unsigned>(color.b)) + ',' +
			       std::to_string(static_cast<unsigned>(color.a));
		}

		bool ParseColor(const char* text, sf::Color& out) {
			if (!text || !*text) {
				return false;
			}
			unsigned r = 0;
			unsigned g = 0;
			unsigned b = 0;
			unsigned a = 255;
			const int matched = std::sscanf(text, "%u,%u,%u,%u", &r, &g, &b, &a);
			if (matched < 3) {
				return false;
			}
			out = sf::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b),
			    static_cast<std::uint8_t>(matched >= 4 ? a : 255));
			return true;
		}

		const char* GridLayoutToString(FieldGridLayout layout) {
			switch (layout) {
			case FieldGridLayout::Square:
				return "Square";
			case FieldGridLayout::Triangular:
				return "Triangular";
			}
			return "Triangular";
		}

		bool TryParseGridLayout(const char* text, FieldGridLayout& out) {
			if (!text || !*text) {
				return false;
			}
			if (std::string_view{text} == "Square") {
				out = FieldGridLayout::Square;
				return true;
			}
			if (std::string_view{text} == "Triangular") {
				out = FieldGridLayout::Triangular;
				return true;
			}
			return false;
		}

		void WriteVelocityNode(pugi::xml_node parent, const PhysicsVisualizer& viz) {
			pugi::xml_node node = parent.append_child(kVelocityElement);
			node.append_attribute(kVisibleAttr).set_value(viz.IsVelocityVisible());
			node.append_attribute(kScaleAttr).set_value(viz.GetVelocityScale());
			node.append_attribute(kColorAttr).set_value(FormatColor(viz.GetVelocityColor()).c_str());
		}

		void WriteForceNode(pugi::xml_node parent, const PhysicsVisualizer& viz) {
			pugi::xml_node node = parent.append_child(kForceElement);
			node.append_attribute(kVisibleAttr).set_value(viz.IsForceVisible());
			node.append_attribute(kScaleAttr).set_value(viz.GetForceScale());
			node.append_attribute(kColorAttr).set_value(FormatColor(viz.GetForceColor()).c_str());
		}

		void WriteAttractionFieldNode(pugi::xml_node parent, const PhysicsVisualizer& viz) {
			pugi::xml_node node = parent.append_child(kAttractionFieldElement);
			node.append_attribute(kVisibleAttr).set_value(viz.IsAttractionFieldVisible());
			node.append_attribute(kGridLayoutAttr).set_value(GridLayoutToString(viz.GetAttractionFieldGridLayout()));
			node.append_attribute(kSpacingPxAttr).set_value(viz.GetAttractionFieldSpacingPx());
			node.append_attribute(kArrowScaleAttr).set_value(viz.GetAttractionFieldArrowScale());
			node.append_attribute(kArrowLengthSoftCapAttr).set_value(viz.GetAttractionFieldArrowLengthSoftCap());
			node.append_attribute(kArrowLengthCompressAttr).set_value(viz.GetAttractionFieldArrowLengthCompress());
			node.append_attribute(kPaletteSpanAttr).set_value(viz.GetAttractionFieldPaletteSpan());
			node.append_attribute(kPaletteWeakAttr).set_value(FormatColor(viz.GetAttractionFieldPaletteWeak()).c_str());
			node.append_attribute(kPaletteMidAttr).set_value(FormatColor(viz.GetAttractionFieldPaletteMid()).c_str());
			node.append_attribute(kPaletteStrongAttr)
			    .set_value(FormatColor(viz.GetAttractionFieldPaletteStrong()).c_str());
			node.append_attribute(kProbeGroupsAttr)
			    .set_value(static_cast<unsigned>(viz.GetAttractionFieldProbeGroups().to_ulong()));
		}

		void LoadVelocityNode(const pugi::xml_node& parent, PhysicsVisualizer& viz) {
			const pugi::xml_node node = parent.child(kVelocityElement);
			if (!node) {
				return;
			}
			if (const pugi::xml_attribute attr = node.attribute(kVisibleAttr)) {
				viz.SetVelocityVisible(attr.as_bool());
			}
			if (const pugi::xml_attribute attr = node.attribute(kScaleAttr)) {
				viz.SetVelocityScale(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kColorAttr)) {
				sf::Color color{};
				if (ParseColor(attr.as_string(), color)) {
					viz.SetVelocityColor(color);
				}
			}
		}

		void LoadForceNode(const pugi::xml_node& parent, PhysicsVisualizer& viz) {
			const pugi::xml_node node = parent.child(kForceElement);
			if (!node) {
				return;
			}
			if (const pugi::xml_attribute attr = node.attribute(kVisibleAttr)) {
				viz.SetForceVisible(attr.as_bool());
			}
			if (const pugi::xml_attribute attr = node.attribute(kScaleAttr)) {
				viz.SetForceScale(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kColorAttr)) {
				sf::Color color{};
				if (ParseColor(attr.as_string(), color)) {
					viz.SetForceColor(color);
				}
			}
		}

		void LoadAttractionFieldNode(
		    const pugi::xml_node& parent, PhysicsVisualizer& viz, SerializationResult& result) {
			const pugi::xml_node node = parent.child(kAttractionFieldElement);
			if (!node) {
				return;
			}
			if (const pugi::xml_attribute attr = node.attribute(kVisibleAttr)) {
				viz.SetAttractionFieldVisible(attr.as_bool());
			}
			if (const pugi::xml_attribute attr = node.attribute(kGridLayoutAttr)) {
				FieldGridLayout layout{};
				if (TryParseGridLayout(attr.as_string(), layout)) {
					viz.SetAttractionFieldGridLayout(layout);
				}
				else {
					result.AddWarning(kGridLayoutAttr, "Unknown attraction field grid layout");
				}
			}
			if (const pugi::xml_attribute attr = node.attribute(kSpacingPxAttr)) {
				viz.SetAttractionFieldSpacingPx(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kArrowScaleAttr)) {
				viz.SetAttractionFieldArrowScale(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kArrowLengthSoftCapAttr)) {
				viz.SetAttractionFieldArrowLengthSoftCap(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kArrowLengthCompressAttr)) {
				viz.SetAttractionFieldArrowLengthCompress(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kPaletteSpanAttr)) {
				viz.SetAttractionFieldPaletteSpan(attr.as_float());
			}
			if (const pugi::xml_attribute attr = node.attribute(kPaletteWeakAttr)) {
				sf::Color color{};
				if (ParseColor(attr.as_string(), color)) {
					viz.SetAttractionFieldPaletteWeak(color);
				}
			}
			if (const pugi::xml_attribute attr = node.attribute(kPaletteMidAttr)) {
				sf::Color color{};
				if (ParseColor(attr.as_string(), color)) {
					viz.SetAttractionFieldPaletteMid(color);
				}
			}
			if (const pugi::xml_attribute attr = node.attribute(kPaletteStrongAttr)) {
				sf::Color color{};
				if (ParseColor(attr.as_string(), color)) {
					viz.SetAttractionFieldPaletteStrong(color);
				}
			}
			if (const pugi::xml_attribute attr = node.attribute(kProbeGroupsAttr)) {
				PhysicsBodyBehaviour::GroupSet groups;
				groups = attr.as_ullong();
				viz.SetAttractionFieldProbeGroups(groups);
			}
		}

		void ApplyVisualizerDefaults(PhysicsVisualizer& viz) {
			viz.SetVelocityVisible(false);
			viz.SetForceVisible(false);
			viz.SetVelocityScale(0.5f);
			viz.SetForceScale(0.5f);
			viz.SetVelocityColor(sf::Color(100, 220, 120));
			viz.SetForceColor(sf::Color(255, 160, 60));

			viz.SetAttractionFieldVisible(false);
			viz.SetAttractionFieldGridLayout(FieldGridLayout::Triangular);
			viz.SetAttractionFieldSpacingPx(48.f);
			viz.SetAttractionFieldArrowScale(0.5f);
			viz.SetAttractionFieldArrowLengthSoftCap(100.f);
			viz.SetAttractionFieldArrowLengthCompress(1.f);
			viz.SetAttractionFieldPaletteSpan(20000.f);
			viz.SetAttractionFieldPaletteWeak(sf::Color(100, 150, 255, 0));
			viz.SetAttractionFieldPaletteMid(sf::Color(220, 220, 100, 150));
			viz.SetAttractionFieldPaletteStrong(sf::Color(255, 80, 50, 200));
			viz.SetAttractionFieldProbeGroups(
			    PhysicsBodyBehaviour::GroupSet{((1ULL << PhysicsBodyBehaviour::kGroupsCount) - 1ULL)});
		}

		class VisualizationSettingsModule final : public ISceneSettingsModule
		{
		public:
			[[nodiscard]] std::string_view GetElementName() const override {
				return kElementName;
			}

			void Save(pugi::xml_node settingsParent, SerializationResult& /*result*/) const override {
				const PhysicsVisualizer& viz = Editor::GetInstance().GetPhysicsVisualizer();
				pugi::xml_node node = settingsParent.append_child(kElementName);
				WriteVelocityNode(node, viz);
				WriteForceNode(node, viz);
				WriteAttractionFieldNode(node, viz);
			}

			void Load(const pugi::xml_node& settingsParent, SerializationResult& result) const override {
				const pugi::xml_node node = settingsParent.child(kElementName);
				if (!node) {
					return;
				}
				PhysicsVisualizer& viz = Editor::GetInstance().GetPhysicsVisualizer();
				LoadVelocityNode(node, viz);
				LoadForceNode(node, viz);
				LoadAttractionFieldNode(node, viz, result);
			}

			void ApplyDefaults() const override {
				ApplyVisualizerDefaults(Editor::GetInstance().GetPhysicsVisualizer());
			}
		};

		const VisualizationSettingsModule kInstance{};

	} // namespace

	const ISceneSettingsModule& GetVisualizationSettingsModule() {
		return kInstance;
	}

} // namespace Engine::Serialization
