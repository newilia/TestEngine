#include "Engine/Editor/PropertyTreeDrawer.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/PropertyNode.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Engine {
	namespace {

		constexpr float kLabelColumnWidth = 160.f;

		void ItemTooltipAfter(const PropertyMeta& meta) {
			if (!meta.tooltip.empty()) {
				ImGui::SetItemTooltip("%s", meta.tooltip.c_str());
			}
		}

		const char* MixedMarker(const PropertyMeta& meta) {
			return meta.mixedValueMarker.empty() ? "mixed" : meta.mixedValueMarker.c_str();
		}

		void PushMixedFlagIfNeeded(const PropertyMeta& meta) {
			if (meta.hasMixedValues) {
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
			}
		}

		void PopMixedFlagIfNeeded(const PropertyMeta& meta) {
			if (meta.hasMixedValues) {
				ImGui::PopItemFlag();
			}
		}

		std::string Trim(std::string value) {
			const auto isSpace = [](unsigned char c) {
				return std::isspace(c) != 0;
			};
			value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char c) {
				return !isSpace(c);
			}));
			value.erase(std::find_if(value.rbegin(), value.rend(),
			                [&](unsigned char c) {
				                return !isSpace(c);
			                })
			                .base(),
			    value.end());
			return value;
		}

		std::vector<std::string> SplitComponentMarkers(const std::string& marker, int expectedCount) {
			std::vector<std::string> result;
			if (marker.empty() || expectedCount <= 0) {
				return result;
			}

			const char separator = marker.find('|') != std::string::npos ? '|' : ',';
			std::stringstream stream(marker);
			std::string token;
			while (std::getline(stream, token, separator)) {
				token = Trim(token);
				const auto colonPos = token.find(':');
				if (colonPos != std::string::npos) {
					token = Trim(token.substr(colonPos + 1));
				}
				if (!token.empty()) {
					result.push_back(token);
				}
			}

			if (static_cast<int>(result.size()) != expectedCount) {
				result.clear();
			}
			return result;
		}

		void DrawLabelLeft(const PropertyNode& n) {
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(n.label.c_str());
			ImGui::SameLine(kLabelColumnWidth);
			ImGui::SetNextItemWidth(std::max(80.f, ImGui::GetContentRegionAvail().x));
		}

		int ClampIntMeta(int v, const PropertyMeta& meta) {
			if (meta.minElementCount) {
				v = std::max(v, static_cast<int>(*meta.minElementCount));
			}
			if (meta.maxElementCount) {
				v = std::min(v, static_cast<int>(*meta.maxElementCount));
			}
			return v;
		}

		bool AsciiContainsCaseInsensitive(std::string_view haystack, std::string_view needle) {
			if (needle.empty()) {
				return true;
			}
			if (needle.size() > haystack.size()) {
				return false;
			}
			const auto lower = [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			};
			for (std::size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
				bool match = true;
				for (std::size_t j = 0; j < needle.size(); ++j) {
					if (lower(static_cast<unsigned char>(haystack[i + j])) !=
					    lower(static_cast<unsigned char>(needle[j]))) {
						match = false;
						break;
					}
				}
				if (match) {
					return true;
				}
			}
			return false;
		}

		bool DrawFilteredValuesProviderCombo(std::array<char, 128>& filter, const char* preview, int& selectedIndex,
		    const std::vector<const char*>& labels) {
			if (!ImGui::BeginCombo("##v", preview)) {
				return false;
			}
			if (ImGui::IsWindowAppearing()) {
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::SetNextItemWidth(-FLT_MIN);
			(void)ImGui::InputTextWithHint("##values_provider_filter", "Filter...", filter.data(), filter.size());
			int visibleCount = 0;
			bool changed = false;
			for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
				if (!AsciiContainsCaseInsensitive(labels[static_cast<std::size_t>(i)], filter.data())) {
					continue;
				}
				++visibleCount;
				const bool isSelected = (selectedIndex == i);
				if (ImGui::Selectable(labels[static_cast<std::size_t>(i)], isSelected)) {
					selectedIndex = i;
					changed = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			if (visibleCount == 0) {
				ImGui::TextDisabled("No matches");
			}
			ImGui::EndCombo();
			if (changed || ImGui::IsItemDeactivated()) {
				filter[0] = '\0';
			}
			return changed;
		}

		void MarkLeafEditedIfMixed(const PropertyNode& node, const PropertyTreeDrawOptions& options) {
			if (options.onPropertyEdited) {
				options.onPropertyEdited();
			}
			if (options.anyLeafEdited == nullptr || !node.meta.hasMixedValues) {
				return;
			}
			*options.anyLeafEdited = true;
		}

		std::shared_ptr<Scene> ResolveSceneForRefs(const PropertyTreeDrawOptions& options) {
			if (options.sceneForSceneRefsOverride) {
				return options.sceneForSceneRefsOverride;
			}
			return MainContext::GetInstance().GetScene();
		}

		void DrawSceneRefEntityLeaf(const char* label, const std::shared_ptr<EntityOnNode>& entity,
		    const PropAccessSceneRef* access, std::uint32_t selectedId,
		    bool (*predicate)(const std::shared_ptr<EntityOnNode>&)) {
			if (!entity) {
				return;
			}
			if (predicate && !predicate(entity)) {
				return;
			}
			const bool selected = (entity->GetSceneObjectId() == selectedId);
			if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_None)) {
				access->set(entity->GetSceneObjectId());
				ImGui::CloseCurrentPopup();
			}
		}

		void DrawSceneRefPickerContent(const Scene& scene, const PropertyNode& n, const PropAccessSceneRef* access) {
			const auto root = scene.GetRoot();
			if (!root) {
				ImGui::TextUnformatted("Scene has no root");
				return;
			}

			const std::uint32_t selectedId = access->get();
			const std::function<void(const std::shared_ptr<SceneNode>&, int)> visit =
			    [&](const std::shared_ptr<SceneNode>& node, int depth) {
				    if (!node) {
					    return;
				    }
				    ImGui::PushID(node.get());
				    const std::string& nm = node->GetName();
				    const char* disp = nm.empty() ? "<unnamed>" : nm.c_str();
				    const std::uint32_t nodeObjectId = node->GetSceneObjectId();

				    if (n.meta.sceneRefFilterKind == SceneRefFilterKind::SceneNode) {
					    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					    if (nodeObjectId == selectedId) {
						    flags |= ImGuiTreeNodeFlags_Selected;
					    }
					    if (depth == 0) {
						    flags |= ImGuiTreeNodeFlags_DefaultOpen;
					    }
					    const bool hasSceneChildren = !node->GetChildren().empty();
					    if (!hasSceneChildren) {
						    flags |= ImGuiTreeNodeFlags_Leaf;
					    }
					    const bool open = ImGui::TreeNodeEx("##sn", flags, "%s", disp);
					    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						    access->set(nodeObjectId);
						    ImGui::CloseCurrentPopup();
					    }
					    if (open) {
						    for (const auto& child : node->GetChildren()) {
							    visit(child, depth + 1);
						    }
						    ImGui::TreePop();
					    }
				    }
				    else {
					    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					    if (depth == 0) {
						    flags |= ImGuiTreeNodeFlags_DefaultOpen;
					    }
					    const bool hasSceneChildren = !node->GetChildren().empty();
					    bool hasEntityChildren = node->GetVisual() || node->GetSortingStrategy();
					    if (!hasEntityChildren) {
						    for (const auto& b : node->GetBehaviours()) {
							    if (b) {
								    hasEntityChildren = true;
								    break;
							    }
						    }
					    }
					    if (!hasSceneChildren && !hasEntityChildren) {
						    flags |= ImGuiTreeNodeFlags_Leaf;
					    }
					    const bool open = ImGui::TreeNodeEx("##sn", flags, "%s", disp);
					    if (open) {
						    if (const auto visual = node->GetVisual()) {
							    DrawSceneRefEntityLeaf("Visual", std::static_pointer_cast<EntityOnNode>(visual), access,
							        selectedId, n.meta.sceneRefEntityIsAllowed);
						    }
						    if (const auto sorting = node->GetSortingStrategy()) {
							    DrawSceneRefEntityLeaf("Sorting", std::static_pointer_cast<EntityOnNode>(sorting),
							        access, selectedId, n.meta.sceneRefEntityIsAllowed);
						    }
						    int behaviourIndex = 0;
						    for (const auto& behaviour : node->GetBehaviours()) {
							    if (!behaviour) {
								    continue;
							    }
							    ImGui::PushID(behaviour.get());
							    Behaviour* const bp = behaviour.get();
							    const std::string bl =
							        fmt::format("Behaviour #{} ({})", behaviourIndex, typeid(*bp).name());
							    DrawSceneRefEntityLeaf(bl.c_str(), std::static_pointer_cast<EntityOnNode>(behaviour),
							        access, selectedId, n.meta.sceneRefEntityIsAllowed);
							    ImGui::PopID();
							    ++behaviourIndex;
						    }
						    for (const auto& child : node->GetChildren()) {
							    visit(child, depth + 1);
						    }
						    ImGui::TreePop();
					    }
				    }

				    ImGui::PopID();
			    };

			visit(root, 0);

			if (ImGui::SmallButton("Clear reference")) {
				access->set(0);
				ImGui::CloseCurrentPopup();
			}
		}

	} // namespace

	void PropertyTreeDrawer::Draw(const PropertyTree& tree, PropertyTreeDrawOptions options) const {
		if (options.unwrapSingleRootObject && tree.roots.size() == 1) {
			const PropertyNode& root = tree.roots.front();
			if (root.kind == PropertyKind::Object) {
				if (root.children.empty()) {
					DrawNode(root, options);
				}
				else {
					for (const PropertyNode& child : root.children) {
						DrawNode(child, options);
					}
				}
				return;
			}
		}
		for (const auto& root : tree.roots) {
			DrawNode(root, options);
		}
	}

	void PropertyTreeDrawer::DrawNode(const PropertyNode& n, PropertyTreeDrawOptions drawOptions) const {
		ImGui::PushID(n.id.c_str());

		const bool readOnly = n.meta.readOnly;

		switch (n.kind) {
		case PropertyKind::Object: {
			if (n.children.empty()) {
				ImGui::TextUnformatted(n.label.c_str());
			}
			else if (ImGui::TreeNodeEx("##obj", 0, "%s", n.label.c_str())) {
				for (const auto& c : n.children) {
					DrawNode(c, drawOptions);
				}
				ImGui::TreePop();
			}
			ItemTooltipAfter(n.meta);
			break;
		}
		case PropertyKind::Sequence: {
			const auto* seq = std::get_if<PropAccessSequence>(&n.access);
			if (ImGui::TreeNodeEx("##seq", 0, "%s", n.label.c_str())) {
				if (seq && seq->getSize) {
					const int sz = static_cast<int>(seq->getSize());
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Size = %d", sz);
					if (seq->resize && !readOnly) {
						ImGui::SameLine();
						ImGui::PushID("szctl");
						if (ImGui::SmallButton("-")) {
							const int next = ClampIntMeta(sz - 1, n.meta);
							if (next != sz) {
								seq->resize(static_cast<std::size_t>(next));
								MarkLeafEditedIfMixed(n, drawOptions);
							}
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("+")) {
							const int next = ClampIntMeta(sz + 1, n.meta);
							if (next != sz) {
								seq->resize(static_cast<std::size_t>(next));
								MarkLeafEditedIfMixed(n, drawOptions);
							}
						}
						ImGui::PopID();
					}
					ItemTooltipAfter(n.meta);
				}
				for (const auto& c : n.children) {
					DrawNode(c, drawOptions);
				}
				ImGui::TreePop();
			}
			else {
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Associative: {
			const auto* asc = std::get_if<PropAccessAssociative>(&n.access);
			if (ImGui::TreeNodeEx("##asc", 0, "%s", n.label.c_str())) {
				if (asc && asc->addPair && !readOnly) {
					if (ImGui::Button("Add entry")) {
						asc->addPair();
						MarkLeafEditedIfMixed(n, drawOptions);
					}
					ItemTooltipAfter(n.meta);
				}
				for (std::size_t i = 0; i < n.children.size(); ++i) {
					ImGui::PushID(static_cast<int>(i));
					if (asc && asc->removePair && !readOnly) {
						if (ImGui::SmallButton("Remove")) {
							asc->removePair(i);
							MarkLeafEditedIfMixed(n, drawOptions);
							ImGui::PopID();
							ImGui::TreePop();
							ImGui::PopID();
							return;
						}
						ImGui::SameLine();
					}
					DrawNode(n.children[i], drawOptions);
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			else {
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Bool: {
			if (const auto* a = std::get_if<PropAccessBool>(&n.access)) {
				DrawLabelLeft(n);
				bool v = a->get();
				if (readOnly) {
					ImGui::TextUnformatted(n.meta.hasMixedValues ? MixedMarker(n.meta) : (v ? "true" : "false"));
				}
				else {
					PushMixedFlagIfNeeded(n.meta);
					if (ImGui::Checkbox("##v", &v)) {
						a->set(v);
						MarkLeafEditedIfMixed(n, drawOptions);
					}
					PopMixedFlagIfNeeded(n.meta);
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Int32: {
			if (const auto* a = std::get_if<PropAccessInt32>(&n.access)) {
				DrawLabelLeft(n);
				if (n.meta.valuesProviderInt32) {
					std::vector<std::int32_t> storage = n.meta.valuesProviderInt32();
					const std::int32_t cur = a->get();
					if (std::find(storage.begin(), storage.end(), cur) == storage.end()) {
						storage.insert(storage.begin(), cur);
					}
					std::vector<std::string> labelStorage;
					labelStorage.reserve(storage.size());
					for (const std::int32_t iv : storage) {
						labelStorage.push_back(std::to_string(iv));
					}
					std::vector<const char*> labels;
					labels.reserve(labelStorage.size());
					for (const std::string& ls : labelStorage) {
						labels.push_back(ls.c_str());
					}
					int idx = 0;
					for (std::size_t i = 0; i < storage.size(); ++i) {
						if (storage[i] == cur) {
							idx = static_cast<int>(i);
							break;
						}
					}
					if (readOnly) {
						const char* text =
						    n.meta.hasMixedValues ? MixedMarker(n.meta) : labels[static_cast<std::size_t>(idx)];
						ImGui::TextUnformatted(text);
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						if (DrawFilteredValuesProviderCombo(
						        _valuesProviderComboFilter, labels[static_cast<std::size_t>(idx)], idx, labels)) {
							a->set(storage[static_cast<std::size_t>(idx)]);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				else {
					int v = static_cast<int>(a->get());
					if (readOnly) {
						if (n.meta.hasMixedValues) {
							ImGui::TextUnformatted(MixedMarker(n.meta));
						}
						else {
							ImGui::Text("%d", v);
						}
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						const char* format = n.meta.hasMixedValues ? MixedMarker(n.meta) : "%d";
						if (ImGui::DragInt("##v", &v, 1.0f, 0, 0, format)) {
							a->set(static_cast<std::int32_t>(v));
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Int64: {
			if (const auto* a = std::get_if<PropAccessInt64>(&n.access)) {
				DrawLabelLeft(n);
				if (n.meta.valuesProviderInt64) {
					std::vector<std::int64_t> storage = n.meta.valuesProviderInt64();
					const std::int64_t cur = a->get();
					if (std::find(storage.begin(), storage.end(), cur) == storage.end()) {
						storage.insert(storage.begin(), cur);
					}
					std::vector<std::string> labelStorage;
					labelStorage.reserve(storage.size());
					for (const std::int64_t iv : storage) {
						labelStorage.push_back(std::to_string(iv));
					}
					std::vector<const char*> labels;
					labels.reserve(labelStorage.size());
					for (const std::string& ls : labelStorage) {
						labels.push_back(ls.c_str());
					}
					int idx = 0;
					for (std::size_t i = 0; i < storage.size(); ++i) {
						if (storage[i] == cur) {
							idx = static_cast<int>(i);
							break;
						}
					}
					if (readOnly) {
						const char* text =
						    n.meta.hasMixedValues ? MixedMarker(n.meta) : labels[static_cast<std::size_t>(idx)];
						ImGui::TextUnformatted(text);
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						if (DrawFilteredValuesProviderCombo(
						        _valuesProviderComboFilter, labels[static_cast<std::size_t>(idx)], idx, labels)) {
							a->set(storage[static_cast<std::size_t>(idx)]);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				else {
					long long v = static_cast<long long>(a->get());
					if (readOnly) {
						if (n.meta.hasMixedValues) {
							ImGui::TextUnformatted(MixedMarker(n.meta));
						}
						else {
							ImGui::Text("%lld", static_cast<long long>(v));
						}
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						const char* format = n.meta.hasMixedValues ? MixedMarker(n.meta) : "%lld";
						if (ImGui::DragScalar("##v", ImGuiDataType_S64, &v, 1.0f, nullptr, nullptr, format)) {
							a->set(static_cast<std::int64_t>(v));
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Float: {
			if (const auto* a = std::get_if<PropAccessFloat>(&n.access)) {
				DrawLabelLeft(n);
				if (n.meta.valuesProviderFloat) {
					std::vector<float> storage = n.meta.valuesProviderFloat();
					const float cur = a->get();
					if (std::find(storage.begin(), storage.end(), cur) == storage.end()) {
						storage.insert(storage.begin(), cur);
					}
					std::vector<std::string> labelStorage;
					labelStorage.reserve(storage.size());
					for (const float fv : storage) {
						labelStorage.push_back(std::to_string(fv));
					}
					std::vector<const char*> labels;
					labels.reserve(labelStorage.size());
					for (const std::string& ls : labelStorage) {
						labels.push_back(ls.c_str());
					}
					int idx = 0;
					for (std::size_t i = 0; i < storage.size(); ++i) {
						if (storage[i] == cur) {
							idx = static_cast<int>(i);
							break;
						}
					}
					if (readOnly) {
						const char* text =
						    n.meta.hasMixedValues ? MixedMarker(n.meta) : labels[static_cast<std::size_t>(idx)];
						ImGui::TextUnformatted(text);
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						if (DrawFilteredValuesProviderCombo(
						        _valuesProviderComboFilter, labels[static_cast<std::size_t>(idx)], idx, labels)) {
							a->set(storage[static_cast<std::size_t>(idx)]);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				else {
					float v = a->get();
					if (readOnly) {
						if (n.meta.hasMixedValues) {
							ImGui::TextUnformatted(MixedMarker(n.meta));
						}
						else {
							ImGui::Text("%.4f", static_cast<double>(v));
						}
					}
					else {
						const float speed = n.meta.dragSpeed.has_value() ? static_cast<float>(*n.meta.dragSpeed) : 1.f;
						PushMixedFlagIfNeeded(n.meta);
						const char* format = n.meta.hasMixedValues ? MixedMarker(n.meta) : "%.3f";
						if (ImGui::DragFloat("##v", &v, speed, 0.0f, 0.0f, format)) {
							if (n.meta.numericMin) {
								v = std::max(v, static_cast<float>(*n.meta.numericMin));
							}
							if (n.meta.numericMax) {
								v = std::min(v, static_cast<float>(*n.meta.numericMax));
							}
							a->set(v);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Double: {
			if (const auto* a = std::get_if<PropAccessDouble>(&n.access)) {
				DrawLabelLeft(n);
				if (n.meta.valuesProviderDouble) {
					std::vector<double> storage = n.meta.valuesProviderDouble();
					const double cur = a->get();
					if (std::find(storage.begin(), storage.end(), cur) == storage.end()) {
						storage.insert(storage.begin(), cur);
					}
					std::vector<std::string> labelStorage;
					labelStorage.reserve(storage.size());
					for (const double dv : storage) {
						labelStorage.push_back(std::to_string(dv));
					}
					std::vector<const char*> labels;
					labels.reserve(labelStorage.size());
					for (const std::string& ls : labelStorage) {
						labels.push_back(ls.c_str());
					}
					int idx = 0;
					for (std::size_t i = 0; i < storage.size(); ++i) {
						if (storage[i] == cur) {
							idx = static_cast<int>(i);
							break;
						}
					}
					if (readOnly) {
						const char* text =
						    n.meta.hasMixedValues ? MixedMarker(n.meta) : labels[static_cast<std::size_t>(idx)];
						ImGui::TextUnformatted(text);
					}
					else {
						PushMixedFlagIfNeeded(n.meta);
						if (DrawFilteredValuesProviderCombo(
						        _valuesProviderComboFilter, labels[static_cast<std::size_t>(idx)], idx, labels)) {
							a->set(storage[static_cast<std::size_t>(idx)]);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				else {
					double v = a->get();
					if (readOnly) {
						if (n.meta.hasMixedValues) {
							ImGui::TextUnformatted(MixedMarker(n.meta));
						}
						else {
							ImGui::Text("%.6f", v);
						}
					}
					else {
						const float speed =
						    n.meta.dragSpeed.has_value() ? static_cast<float>(*n.meta.dragSpeed) : 0.05f;
						PushMixedFlagIfNeeded(n.meta);
						const char* format = n.meta.hasMixedValues ? MixedMarker(n.meta) : "%.6f";
						if (ImGui::DragScalar("##v", ImGuiDataType_Double, &v, speed, nullptr, nullptr, format)) {
							if (n.meta.numericMin) {
								v = std::max(v, *n.meta.numericMin);
							}
							if (n.meta.numericMax) {
								v = std::min(v, *n.meta.numericMax);
							}
							a->set(v);
							MarkLeafEditedIfMixed(n, drawOptions);
						}
						PopMixedFlagIfNeeded(n.meta);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::String: {
			if (const auto* a = std::get_if<PropAccessString>(&n.access)) {
				DrawLabelLeft(n);
				std::string s = a->get();
				bool drewCombo = false;
				if (n.meta.valuesProviderStdString && !n.meta.stringMultiline) {
					std::vector<std::string> storage = n.meta.valuesProviderStdString();
					if (!storage.empty()) {
						drewCombo = true;
						if (std::find(storage.begin(), storage.end(), s) == storage.end()) {
							storage.insert(storage.begin(), s);
						}
						std::vector<const char*> labels;
						labels.reserve(storage.size());
						for (const std::string& row : storage) {
							if (row.empty()) {
								labels.push_back("##empty");
							}
							else {
								labels.push_back(row.c_str());
							}
						}
						int idx = 0;
						for (std::size_t i = 0; i < storage.size(); ++i) {
							if (storage[i] == s) {
								idx = static_cast<int>(i);
								break;
							}
						}
						if (readOnly) {
							const char* text =
							    n.meta.hasMixedValues ? MixedMarker(n.meta) : labels[static_cast<std::size_t>(idx)];
							ImGui::TextUnformatted(text);
						}
						else {
							PushMixedFlagIfNeeded(n.meta);
							if (DrawFilteredValuesProviderCombo(
							        _valuesProviderComboFilter, labels[static_cast<std::size_t>(idx)], idx, labels)) {
								a->set(storage[static_cast<std::size_t>(idx)]);
								MarkLeafEditedIfMixed(n, drawOptions);
							}
							PopMixedFlagIfNeeded(n.meta);
						}
					}
				}
				if (!drewCombo) {
					std::array<char, 512> buf{};
					(void)std::snprintf(buf.data(), buf.size(), "%s", s.c_str());
					const ImGuiInputTextFlags flags = readOnly ? ImGuiInputTextFlags_ReadOnly : 0;
					bool edited = false;
					if (n.meta.stringMultiline) {
						edited = ImGui::InputTextMultiline("##v", buf.data(), buf.size(), ImVec2(0, 80), flags);
					}
					else if (!readOnly && n.meta.hasMixedValues) {
						edited = ImGui::InputTextWithHint("##v", MixedMarker(n.meta), buf.data(), buf.size(), flags);
					}
					else {
						edited = ImGui::InputText("##v", buf.data(), buf.size(), flags);
					}
					if (edited && !readOnly) {
						a->set(std::string(buf.data()));
						MarkLeafEditedIfMixed(n, drawOptions);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Enum: {
			if (const auto* a = std::get_if<PropAccessEnum>(&n.access)) {
				DrawLabelLeft(n);
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
					const char* text = n.meta.hasMixedValues ? MixedMarker(n.meta) : "<unknown>";
					for (const auto& opt : a->options) {
						if (opt.first == current) {
							text = opt.second.c_str();
							break;
						}
					}
					ImGui::TextUnformatted(text);
				}
				else if (!labels.empty()) {
					PushMixedFlagIfNeeded(n.meta);
					if (ImGui::Combo("##v", &idx, labels.data(), static_cast<int>(labels.size()))) {
						a->set(a->options[static_cast<std::size_t>(idx)].first);
						MarkLeafEditedIfMixed(n, drawOptions);
					}
					PopMixedFlagIfNeeded(n.meta);
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2f: {
			if (const auto* a = std::get_if<PropAccessVec2f>(&n.access)) {
				DrawLabelLeft(n);
				sf::Vector2f v = a->get();
				float arr[2] = {v.x, v.y};
				if (readOnly) {
					if (n.meta.hasMixedValues) {
						ImGui::TextUnformatted(MixedMarker(n.meta));
					}
					else {
						ImGui::Text("(%.2f, %.2f)", static_cast<double>(v.x), static_cast<double>(v.y));
					}
				}
				else {
					bool changed = false;
					const auto componentMarkers = n.meta.hasMixedValues ? SplitComponentMarkers(MixedMarker(n.meta), 2)
					                                                    : std::vector<std::string>{};
					ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
					for (int i = 0; i < 2; ++i) {
						ImGui::PushID(i);
						if (i > 0) {
							ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
						}
						PushMixedFlagIfNeeded(n.meta);
						const char* format =
						    componentMarkers.empty() ? "%.3f" : componentMarkers[static_cast<std::size_t>(i)].c_str();
						changed |= ImGui::DragFloat("##v", &arr[i], n.meta.dragSpeed.value_or(1.f), 0.0f, 0.0f, format);
						PopMixedFlagIfNeeded(n.meta);
						ImGui::PopID();
						ImGui::PopItemWidth();
					}
					if (changed) {
						a->set(sf::Vector2f{arr[0], arr[1]});
						MarkLeafEditedIfMixed(n, drawOptions);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2i: {
			if (const auto* a = std::get_if<PropAccessVec2i>(&n.access)) {
				DrawLabelLeft(n);
				sf::Vector2i v = a->get();
				int arr[2] = {v.x, v.y};
				if (readOnly) {
					if (n.meta.hasMixedValues) {
						ImGui::TextUnformatted(MixedMarker(n.meta));
					}
					else {
						ImGui::Text("(%d, %d)", arr[0], arr[1]);
					}
				}
				else {
					bool changed = false;
					const float speed = n.meta.dragSpeed.value_or(1.f);
					const auto componentMarkers = n.meta.hasMixedValues ? SplitComponentMarkers(MixedMarker(n.meta), 2)
					                                                    : std::vector<std::string>{};
					ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
					for (int i = 0; i < 2; ++i) {
						ImGui::PushID(i);
						if (i > 0) {
							ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
						}
						PushMixedFlagIfNeeded(n.meta);
						const char* format =
						    componentMarkers.empty() ? "%d" : componentMarkers[static_cast<std::size_t>(i)].c_str();
						changed |= ImGui::DragInt("##v", &arr[i], speed, 0, 0, format);
						PopMixedFlagIfNeeded(n.meta);
						ImGui::PopID();
						ImGui::PopItemWidth();
					}
					if (changed) {
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
						MarkLeafEditedIfMixed(n, drawOptions);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec2u: {
			if (const auto* a = std::get_if<PropAccessVec2u>(&n.access)) {
				DrawLabelLeft(n);
				sf::Vector2u v = a->get();
				std::uint32_t arr[2] = {static_cast<std::uint32_t>(v.x), static_cast<std::uint32_t>(v.y)};
				if (readOnly) {
					if (n.meta.hasMixedValues) {
						ImGui::TextUnformatted(MixedMarker(n.meta));
					}
					else {
						ImGui::Text("(%u, %u)", static_cast<unsigned>(arr[0]), static_cast<unsigned>(arr[1]));
					}
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
					bool changed = false;
					const auto componentMarkers = n.meta.hasMixedValues ? SplitComponentMarkers(MixedMarker(n.meta), 2)
					                                                    : std::vector<std::string>{};
					ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
					for (int i = 0; i < 2; ++i) {
						ImGui::PushID(i);
						if (i > 0) {
							ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
						}
						PushMixedFlagIfNeeded(n.meta);
						const char* format =
						    componentMarkers.empty() ? "%u" : componentMarkers[static_cast<std::size_t>(i)].c_str();
						changed |= ImGui::DragScalar("##v", ImGuiDataType_U32, &arr[i], speed, pMin, pMax, format, 0);
						PopMixedFlagIfNeeded(n.meta);
						ImGui::PopID();
						ImGui::PopItemWidth();
					}
					if (changed) {
						a->set(sf::Vector2u{static_cast<unsigned int>(arr[0]), static_cast<unsigned int>(arr[1])});
						MarkLeafEditedIfMixed(n, drawOptions);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Vec3f: {
			if (const auto* a = std::get_if<PropAccessVec3f>(&n.access)) {
				DrawLabelLeft(n);
				sf::Vector3f v = a->get();
				float arr[3] = {v.x, v.y, v.z};
				if (readOnly) {
					if (n.meta.hasMixedValues) {
						ImGui::TextUnformatted(MixedMarker(n.meta));
					}
					else {
						ImGui::Text("(%.2f, %.2f, %.2f)", static_cast<double>(v.x), static_cast<double>(v.y),
						    static_cast<double>(v.z));
					}
				}
				else {
					bool changed = false;
					const auto componentMarkers = n.meta.hasMixedValues ? SplitComponentMarkers(MixedMarker(n.meta), 3)
					                                                    : std::vector<std::string>{};
					ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
					for (int i = 0; i < 3; ++i) {
						ImGui::PushID(i);
						if (i > 0) {
							ImGui::SameLine(0.f, ImGui::GetStyle().ItemInnerSpacing.x);
						}
						PushMixedFlagIfNeeded(n.meta);
						const char* format =
						    componentMarkers.empty() ? "%.3f" : componentMarkers[static_cast<std::size_t>(i)].c_str();
						changed |= ImGui::DragFloat("##v", &arr[i], n.meta.dragSpeed.value_or(1.f), 0.0f, 0.0f, format);
						PopMixedFlagIfNeeded(n.meta);
						ImGui::PopID();
						ImGui::PopItemWidth();
					}
					if (changed) {
						a->set(sf::Vector3f{arr[0], arr[1], arr[2]});
						MarkLeafEditedIfMixed(n, drawOptions);
					}
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::Color: {
			if (const auto* a = std::get_if<PropAccessColor>(&n.access)) {
				DrawLabelLeft(n);
				sf::Color c = a->get();
				float rgba[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f};
				if (readOnly) {
					if (n.meta.hasMixedValues) {
						ImGui::TextUnformatted(MixedMarker(n.meta));
					}
					else {
						ImGui::ColorButton("##ro", ImVec4{rgba[0], rgba[1], rgba[2], rgba[3]},
						    ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
					}
				}
				else {
					PushMixedFlagIfNeeded(n.meta);
					if (ImGui::ColorEdit4("##v", rgba, ImGuiColorEditFlags_Float)) {
						const auto clamp255 = [](float x) {
							return static_cast<std::uint8_t>(std::clamp(std::lround(x * 255.f), 0L, 255L));
						};
						a->set(sf::Color{clamp255(rgba[0]), clamp255(rgba[1]), clamp255(rgba[2]), clamp255(rgba[3])});
						MarkLeafEditedIfMixed(n, drawOptions);
					}
					PopMixedFlagIfNeeded(n.meta);
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		case PropertyKind::SceneRef: {
			if (const auto* a = std::get_if<PropAccessSceneRef>(&n.access)) {
				DrawLabelLeft(n);
				const std::shared_ptr<Scene> scene = ResolveSceneForRefs(drawOptions);
				const std::uint32_t cur = a->get();
				std::string summary = "None";
				if (cur != 0) {
					if (scene) {
						if (n.meta.sceneRefFilterKind == SceneRefFilterKind::SceneNode) {
							if (const auto target = scene->FindNodeByObjectId(cur)) {
								const std::string& nm = target->GetName();
								summary = nm.empty() ? fmt::format("Node {}", cur) : nm;
							}
							else {
								summary = fmt::format("Missing {}", cur);
							}
						}
						else if (const auto ent = scene->FindEntityByObjectId(cur)) {
							summary = fmt::format("{} [{}]", typeid(*ent).name(), cur);
						}
						else {
							summary = fmt::format("Missing {}", cur);
						}
					}
					else {
						summary = fmt::format("id {}", cur);
					}
				}
				if (readOnly) {
					ImGui::TextUnformatted(n.meta.hasMixedValues ? MixedMarker(n.meta) : summary.c_str());
				}
				else {
					PushMixedFlagIfNeeded(n.meta);
					ImGui::TextUnformatted(summary.c_str());
					ImGui::SameLine();
					const std::string popupId = fmt::format("SceneRefPick##{}", n.id);
					if (ImGui::Button("Pick…")) {
						ImGui::OpenPopup(popupId.c_str());
					}
					if (ImGui::BeginPopupModal(popupId.c_str(), nullptr,
					        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
						if (!scene) {
							ImGui::TextUnformatted("No active scene");
						}
						else {
							DrawSceneRefPickerContent(*scene, n, a);
						}
						if (ImGui::Button("Close")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					PopMixedFlagIfNeeded(n.meta);
				}
				ItemTooltipAfter(n.meta);
			}
			break;
		}
		}

		ImGui::PopID();
	}
} // namespace Engine
