#include "Engine/Editor/PropertyTreeDrawer.h"

#include "Engine/Core/PropertyNode.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <variant>

namespace Engine {
	namespace {

		constexpr float kLabelColumnWidth = 160.f;

		void itemTooltipAfter(const PropertyMeta& meta) {
			if (!meta.tooltip.empty()) {
				ImGui::SetItemTooltip("%s", meta.tooltip.c_str());
			}
		}

		void drawLabelLeft(const PropertyNode& n) {
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(n.label.c_str());
			ImGui::SameLine(kLabelColumnWidth);
			ImGui::SetNextItemWidth(std::max(80.f, ImGui::GetContentRegionAvail().x));
		}

		int clampIntMeta(int v, const PropertyMeta& meta) {
			if (meta.minElementCount) {
				v = std::max(v, static_cast<int>(*meta.minElementCount));
			}
			if (meta.maxElementCount) {
				v = std::min(v, static_cast<int>(*meta.maxElementCount));
			}
			return v;
		}
	} // namespace

	void PropertyTreeDrawer::Draw(const PropertyTree& tree, PropertyTreeDrawOptions options) const {
		if (options.unwrapSingleRootObject && tree.roots.size() == 1) {
			const PropertyNode& root = tree.roots.front();
			if (root.kind == PropertyKind::Object) {
				if (root.children.empty()) {
					drawNode(root);
				}
				else {
					for (const PropertyNode& child : root.children) {
						drawNode(child);
					}
				}
				return;
			}
		}
		for (const auto& root : tree.roots) {
			drawNode(root);
		}
	}

	void PropertyTreeDrawer::drawNode(const PropertyNode& n) const {
		ImGui::PushID(n.id.c_str());

		const bool readOnly = n.meta.readOnly;

		switch (n.kind) {
		case PropertyKind::Object: {
			if (n.children.empty()) {
				ImGui::TextUnformatted(n.label.c_str());
			}
			else if (ImGui::TreeNodeEx("##obj", ImGuiTreeNodeFlags_DefaultOpen, "%s", n.label.c_str())) {
				for (const auto& c : n.children) {
					drawNode(c);
				}
				ImGui::TreePop();
			}
			itemTooltipAfter(n.meta);
			break;
		}
		case PropertyKind::Sequence: {
			const auto* seq = std::get_if<PropAccessSequence>(&n.access);
			if (ImGui::TreeNodeEx("##seq", ImGuiTreeNodeFlags_DefaultOpen, "%s", n.label.c_str())) {
				if (seq && seq->getSize) {
					const int sz = static_cast<int>(seq->getSize());
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Size = %d", sz);
					if (seq->resize && !readOnly) {
						ImGui::SameLine();
						ImGui::PushID("szctl");
						if (ImGui::SmallButton("-")) {
							const int next = clampIntMeta(sz - 1, n.meta);
							if (next != sz) {
								seq->resize(static_cast<std::size_t>(next));
							}
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("+")) {
							const int next = clampIntMeta(sz + 1, n.meta);
							if (next != sz) {
								seq->resize(static_cast<std::size_t>(next));
							}
						}
						ImGui::PopID();
					}
					itemTooltipAfter(n.meta);
				}
				for (const auto& c : n.children) {
					drawNode(c);
				}
				ImGui::TreePop();
			}
			else {
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Associative: {
			const auto* asc = std::get_if<PropAccessAssociative>(&n.access);
			if (ImGui::TreeNodeEx("##asc", ImGuiTreeNodeFlags_DefaultOpen, "%s", n.label.c_str())) {
				if (asc && asc->addPair && !readOnly) {
					if (ImGui::Button("Add entry")) {
						asc->addPair();
					}
					itemTooltipAfter(n.meta);
				}
				for (std::size_t i = 0; i < n.children.size(); ++i) {
					ImGui::PushID(static_cast<int>(i));
					if (asc && asc->removePair && !readOnly) {
						if (ImGui::SmallButton("Remove")) {
							asc->removePair(i);
							ImGui::PopID();
							ImGui::TreePop();
							ImGui::PopID();
							return;
						}
						ImGui::SameLine();
					}
					drawNode(n.children[i]);
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			else {
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Bool: {
			if (const auto* a = std::get_if<PropAccessBool>(&n.access)) {
				drawLabelLeft(n);
				bool v = a->get();
				if (readOnly) {
					ImGui::TextUnformatted(v ? "true" : "false");
				}
				else if (ImGui::Checkbox("##v", &v)) {
					a->set(v);
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Int32: {
			if (const auto* a = std::get_if<PropAccessInt32>(&n.access)) {
				drawLabelLeft(n);
				int v = static_cast<int>(a->get());
				if (readOnly) {
					ImGui::Text("%d", v);
				}
				else if (ImGui::DragInt("##v", &v)) {
					a->set(static_cast<std::int32_t>(v));
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Int64: {
			if (const auto* a = std::get_if<PropAccessInt64>(&n.access)) {
				drawLabelLeft(n);
				long long v = static_cast<long long>(a->get());
				if (readOnly) {
					ImGui::Text("%lld", static_cast<long long>(v));
				}
				else if (ImGui::DragScalar("##v", ImGuiDataType_S64, &v)) {
					a->set(static_cast<std::int64_t>(v));
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Float: {
			if (const auto* a = std::get_if<PropAccessFloat>(&n.access)) {
				drawLabelLeft(n);
				float v = a->get();
				if (readOnly) {
					ImGui::Text("%.4f", static_cast<double>(v));
				}
				else {
					const float speed = n.meta.dragSpeed.has_value() ? static_cast<float>(*n.meta.dragSpeed) : 1.f;
					if (ImGui::DragFloat("##v", &v, speed)) {
						if (n.meta.numericMin) {
							v = std::max(v, static_cast<float>(*n.meta.numericMin));
						}
						if (n.meta.numericMax) {
							v = std::min(v, static_cast<float>(*n.meta.numericMax));
						}
						a->set(v);
					}
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Double: {
			if (const auto* a = std::get_if<PropAccessDouble>(&n.access)) {
				drawLabelLeft(n);
				double v = a->get();
				if (readOnly) {
					ImGui::Text("%.6f", v);
				}
				else {
					const float speed = n.meta.dragSpeed.has_value() ? static_cast<float>(*n.meta.dragSpeed) : 0.05f;
					if (ImGui::DragScalar("##v", ImGuiDataType_Double, &v, speed)) {
						if (n.meta.numericMin) {
							v = std::max(v, *n.meta.numericMin);
						}
						if (n.meta.numericMax) {
							v = std::min(v, *n.meta.numericMax);
						}
						a->set(v);
					}
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::String: {
			if (const auto* a = std::get_if<PropAccessString>(&n.access)) {
				drawLabelLeft(n);
				std::string s = a->get();
				std::array<char, 512> buf{};
				(void)std::snprintf(buf.data(), buf.size(), "%s", s.c_str());
				const ImGuiInputTextFlags flags = readOnly ? ImGuiInputTextFlags_ReadOnly : 0;
				bool edited = false;
				if (n.meta.stringMultiline) {
					edited = ImGui::InputTextMultiline("##v", buf.data(), buf.size(), ImVec2(0, 80), flags);
				}
				else {
					edited = ImGui::InputText("##v", buf.data(), buf.size(), flags);
				}
				if (edited && !readOnly) {
					a->set(std::string(buf.data()));
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Enum: {
			if (const auto* a = std::get_if<PropAccessEnum>(&n.access)) {
				drawLabelLeft(n);
				int current = a->get();
				int idx = 0;
				for (std::size_t i = 0; i < a->options.size(); ++i) {
					if (a->options[i].first == current) {
						idx = static_cast<int>(i);
						break;
					}
				}
				std::vector<const char*> labels;
				labels.reserve(a->options.size());
				for (const auto& opt : a->options) {
					labels.push_back(opt.second.c_str());
				}
				if (readOnly) {
					const char* text = "<unknown>";
					for (const auto& opt : a->options) {
						if (opt.first == current) {
							text = opt.second.c_str();
							break;
						}
					}
					ImGui::TextUnformatted(text);
				}
				else if (!labels.empty() && ImGui::Combo("##v", &idx, labels.data(), static_cast<int>(labels.size()))) {
					a->set(a->options[static_cast<std::size_t>(idx)].first);
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2f: {
			if (const auto* a = std::get_if<PropAccessVec2f>(&n.access)) {
				drawLabelLeft(n);
				sf::Vector2f v = a->get();
				float arr[2] = {v.x, v.y};
				if (readOnly) {
					ImGui::Text("(%.2f, %.2f)", static_cast<double>(v.x), static_cast<double>(v.y));
				}
				else if (ImGui::DragFloat2("##v", arr, n.meta.dragSpeed.value_or(1.f))) {
					a->set(sf::Vector2f{arr[0], arr[1]});
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2i: {
			if (const auto* a = std::get_if<PropAccessVec2i>(&n.access)) {
				drawLabelLeft(n);
				sf::Vector2i v = a->get();
				int arr[2] = {v.x, v.y};
				if (readOnly) {
					ImGui::Text("(%d, %d)", arr[0], arr[1]);
				}
				else {
					const float speed = n.meta.dragSpeed.value_or(1.f);
					if (ImGui::DragInt2("##v", arr, speed)) {
						int x = arr[0];
						int y = arr[1];
						if (n.meta.numericMin) {
							const int mn = static_cast<int>(*n.meta.numericMin);
							x = std::max(x, mn);
							y = std::max(y, mn);
						}
						if (n.meta.numericMax) {
							const int mx = static_cast<int>(*n.meta.numericMax);
							x = std::min(x, mx);
							y = std::min(y, mx);
						}
						a->set(sf::Vector2i{x, y});
					}
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2u: {
			if (const auto* a = std::get_if<PropAccessVec2u>(&n.access)) {
				drawLabelLeft(n);
				sf::Vector2u v = a->get();
				std::uint32_t arr[2] = {static_cast<std::uint32_t>(v.x), static_cast<std::uint32_t>(v.y)};
				if (readOnly) {
					ImGui::Text("(%u, %u)", static_cast<unsigned>(arr[0]), static_cast<unsigned>(arr[1]));
				}
				else {
					const float speed = n.meta.dragSpeed.value_or(1.f);
					const void* pMin = nullptr;
					const void* pMax = nullptr;
					std::uint32_t vmin = 0;
					std::uint32_t vmax = 0;
					if (n.meta.numericMin) {
						vmin = static_cast<std::uint32_t>(*n.meta.numericMin);
						pMin = &vmin;
					}
					if (n.meta.numericMax) {
						vmax = static_cast<std::uint32_t>(*n.meta.numericMax);
						pMax = &vmax;
					}
					if (ImGui::DragScalarN("##v", ImGuiDataType_U32, arr, 2, speed, pMin, pMax, "%u", 0)) {
						a->set(sf::Vector2u{static_cast<unsigned int>(arr[0]), static_cast<unsigned int>(arr[1])});
					}
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec3f: {
			if (const auto* a = std::get_if<PropAccessVec3f>(&n.access)) {
				drawLabelLeft(n);
				sf::Vector3f v = a->get();
				float arr[3] = {v.x, v.y, v.z};
				if (readOnly) {
					ImGui::Text("(%.2f, %.2f, %.2f)", static_cast<double>(v.x), static_cast<double>(v.y),
					            static_cast<double>(v.z));
				}
				else if (ImGui::DragFloat3("##v", arr, n.meta.dragSpeed.value_or(1.f))) {
					a->set(sf::Vector3f{arr[0], arr[1], arr[2]});
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Color: {
			if (const auto* a = std::get_if<PropAccessColor>(&n.access)) {
				drawLabelLeft(n);
				sf::Color c = a->get();
				float rgba[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f};
				if (readOnly) {
					ImGui::ColorButton("##ro", ImVec4{rgba[0], rgba[1], rgba[2], rgba[3]},
					                   ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
				}
				else if (ImGui::ColorEdit4("##v", rgba, ImGuiColorEditFlags_Float)) {
					const auto clamp255 = [](float x) {
						return static_cast<std::uint8_t>(std::clamp(std::lround(x * 255.f), 0L, 255L));
					};
					a->set(sf::Color{clamp255(rgba[0]), clamp255(rgba[1]), clamp255(rgba[2]), clamp255(rgba[3])});
				}
				itemTooltipAfter(n.meta);
			}
			break;
		}
		}

		ImGui::PopID();
	}
} // namespace Engine
