#include "Engine/Editor/NodeInspectorWidget.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorVisualTheme.h"
#include "Engine/Editor/SceneCloneUtils.h"

#include <fmt/format.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
	struct PropertyNodeKey
	{
		std::string id;
		Engine::PropertyKind kind{};

		bool operator==(const PropertyNodeKey& other) const {
			return id == other.id && kind == other.kind;
		}
	};

	struct PropertyNodeKeyHash
	{
		std::size_t operator()(const PropertyNodeKey& key) const {
			return std::hash<std::string>{}(key.id) ^ (static_cast<std::size_t>(key.kind) << 1U);
		}
	};

	template <typename T>
	bool AreAllEqual(const std::vector<T>& values) {
		if (values.empty()) {
			return true;
		}
		const T& first = values.front();
		for (std::size_t i = 1; i < values.size(); ++i) {
			if (!(values[i] == first)) {
				return false;
			}
		}
		return true;
	}

	bool AreAllFloatEqual(const std::vector<float>& values) {
		if (values.empty()) {
			return true;
		}
		const float first = values.front();
		for (std::size_t i = 1; i < values.size(); ++i) {
			if (std::fabs(values[i] - first) > 1e-5f) {
				return false;
			}
		}
		return true;
	}

	bool AreAllDoubleEqual(const std::vector<double>& values) {
		if (values.empty()) {
			return true;
		}
		const double first = values.front();
		for (std::size_t i = 1; i < values.size(); ++i) {
			if (std::fabs(values[i] - first) > 1e-8) {
				return false;
			}
		}
		return true;
	}

	template <typename T>
	std::string BuildIntegerRangeMarker(const std::vector<T>& values) {
		if (values.empty()) {
			return "mixed";
		}
		const auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
		return fmt::format("{} .. {}", *minIt, *maxIt);
	}

	std::string BuildFloatRangeMarker(const std::vector<float>& values) {
		if (values.empty()) {
			return "mixed";
		}
		const auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
		return fmt::format("{:.3f} .. {:.3f}", static_cast<double>(*minIt), static_cast<double>(*maxIt));
	}

	std::string BuildDoubleRangeMarker(const std::vector<double>& values) {
		if (values.empty()) {
			return "mixed";
		}
		const auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
		return fmt::format("{:.3f} .. {:.3f}", *minIt, *maxIt);
	}

	std::string BuildVec2fRangeMarker(const std::vector<sf::Vector2f>& values) {
		if (values.empty()) {
			return "mixed";
		}
		float minX = values.front().x;
		float maxX = values.front().x;
		float minY = values.front().y;
		float maxY = values.front().y;
		for (const auto& value : values) {
			minX = std::min(minX, value.x);
			maxX = std::max(maxX, value.x);
			minY = std::min(minY, value.y);
			maxY = std::max(maxY, value.y);
		}
		return fmt::format("x:{:.3f} .. {:.3f}, y:{:.3f} .. {:.3f}", static_cast<double>(minX),
		    static_cast<double>(maxX), static_cast<double>(minY), static_cast<double>(maxY));
	}

	std::string BuildVec2iRangeMarker(const std::vector<sf::Vector2i>& values) {
		if (values.empty()) {
			return "mixed";
		}
		int minX = values.front().x;
		int maxX = values.front().x;
		int minY = values.front().y;
		int maxY = values.front().y;
		for (const auto& value : values) {
			minX = std::min(minX, value.x);
			maxX = std::max(maxX, value.x);
			minY = std::min(minY, value.y);
			maxY = std::max(maxY, value.y);
		}
		return fmt::format("x:{} .. {}, y:{} .. {}", minX, maxX, minY, maxY);
	}

	std::string BuildVec2uRangeMarker(const std::vector<sf::Vector2u>& values) {
		if (values.empty()) {
			return "mixed";
		}
		unsigned minX = values.front().x;
		unsigned maxX = values.front().x;
		unsigned minY = values.front().y;
		unsigned maxY = values.front().y;
		for (const auto& value : values) {
			minX = std::min(minX, value.x);
			maxX = std::max(maxX, value.x);
			minY = std::min(minY, value.y);
			maxY = std::max(maxY, value.y);
		}
		return fmt::format("x:{} .. {}, y:{} .. {}", minX, maxX, minY, maxY);
	}

	std::string BuildVec3fRangeMarker(const std::vector<sf::Vector3f>& values) {
		if (values.empty()) {
			return "mixed";
		}
		float minX = values.front().x;
		float maxX = values.front().x;
		float minY = values.front().y;
		float maxY = values.front().y;
		float minZ = values.front().z;
		float maxZ = values.front().z;
		for (const auto& value : values) {
			minX = std::min(minX, value.x);
			maxX = std::max(maxX, value.x);
			minY = std::min(minY, value.y);
			maxY = std::max(maxY, value.y);
			minZ = std::min(minZ, value.z);
			maxZ = std::max(maxZ, value.z);
		}
		return fmt::format("x:{:.3f} .. {:.3f}, y:{:.3f} .. {:.3f}, z:{:.3f} .. {:.3f}", static_cast<double>(minX),
		    static_cast<double>(maxX), static_cast<double>(minY), static_cast<double>(maxY), static_cast<double>(minZ),
		    static_cast<double>(maxZ));
	}

	template <typename TAccess, typename TValue>
	Engine::PropertyAccess BuildMergedScalarAccess(const std::vector<const Engine::PropertyNode*>& nodes,
	    Engine::PropertyMeta& meta,
	    const std::function<bool(const std::vector<TValue>&)>& equalPredicate = AreAllEqual<TValue>,
	    const std::function<std::string(const std::vector<TValue>&)>& mixedMarkerBuilder = nullptr) {
		std::vector<TAccess> accesses;
		accesses.reserve(nodes.size());
		std::vector<TValue> values;
		values.reserve(nodes.size());
		for (const auto* node : nodes) {
			const auto* access = std::get_if<TAccess>(&node->access);
			if (!access || !access->get) {
				meta.readOnly = true;
				return Engine::PropAccessNone{};
			}
			accesses.push_back(*access);
			values.push_back(access->get());
			if (!access->set) {
				meta.readOnly = true;
			}
		}
		meta.hasMixedValues = !equalPredicate(values);
		meta.mixedValueMarker = "mixed";
		if (meta.hasMixedValues && mixedMarkerBuilder) {
			meta.mixedValueMarker = mixedMarkerBuilder(values);
		}
		meta.readOnly = meta.readOnly || accesses.empty();
		TAccess merged{};
		merged.get = accesses.front().get;
		if (!meta.readOnly) {
			merged.set = [accesses](TValue value) {
				for (const auto& access : accesses) {
					access.set(value);
				}
			};
		}
		return merged;
	}

	std::optional<Engine::PropertyNode> TryBuildMergedNode(const std::vector<const Engine::PropertyNode*>& nodes);

	Engine::PropertyAccess BuildMergedEnumAccess(
	    const std::vector<const Engine::PropertyNode*>& nodes, Engine::PropertyMeta& meta) {
		std::vector<Engine::PropAccessEnum> accesses;
		accesses.reserve(nodes.size());
		std::vector<int> values;
		values.reserve(nodes.size());
		for (const auto* node : nodes) {
			const auto* access = std::get_if<Engine::PropAccessEnum>(&node->access);
			if (!access || !access->get) {
				meta.readOnly = true;
				return Engine::PropAccessNone{};
			}
			accesses.push_back(*access);
			values.push_back(access->get());
			if (!access->set) {
				meta.readOnly = true;
			}
		}
		meta.hasMixedValues = !AreAllEqual(values);
		meta.readOnly = meta.readOnly || accesses.empty();
		Engine::PropAccessEnum merged{};
		merged.get = accesses.front().get;
		merged.options = accesses.front().options;
		if (!meta.readOnly) {
			merged.set = [accesses](int value) {
				for (const auto& access : accesses) {
					access.set(value);
				}
			};
		}
		return merged;
	}

	std::vector<Engine::PropertyNode> BuildMergedChildren(const std::vector<const Engine::PropertyNode*>& parents) {
		std::vector<Engine::PropertyNode> result;
		if (parents.empty()) {
			return result;
		}

		std::vector<std::unordered_map<PropertyNodeKey, const Engine::PropertyNode*, PropertyNodeKeyHash>> childMaps;
		childMaps.reserve(parents.size());
		for (const auto* parent : parents) {
			std::unordered_map<PropertyNodeKey, const Engine::PropertyNode*, PropertyNodeKeyHash> map;
			for (const auto& child : parent->children) {
				map.emplace(PropertyNodeKey{child.id, child.kind}, &child);
			}
			childMaps.push_back(std::move(map));
		}

		for (const auto& child : parents.front()->children) {
			const PropertyNodeKey key{child.id, child.kind};
			std::vector<const Engine::PropertyNode*> matchingNodes;
			matchingNodes.reserve(parents.size());
			matchingNodes.push_back(&child);
			bool existsInAll = true;
			for (std::size_t i = 1; i < childMaps.size(); ++i) {
				const auto it = childMaps[i].find(key);
				if (it == childMaps[i].end()) {
					existsInAll = false;
					break;
				}
				matchingNodes.push_back(it->second);
			}
			if (!existsInAll) {
				continue;
			}
			if (auto merged = TryBuildMergedNode(matchingNodes)) {
				result.push_back(std::move(*merged));
			}
		}
		return result;
	}

	std::optional<Engine::PropertyNode> TryBuildMergedNode(const std::vector<const Engine::PropertyNode*>& nodes) {
		if (nodes.empty()) {
			return std::nullopt;
		}
		const auto* first = nodes.front();
		Engine::PropertyNode merged = *first;
		merged.children = BuildMergedChildren(nodes);
		merged.meta.hasMixedValues = false;

		switch (first->kind) {
		case Engine::PropertyKind::Object:
			merged.access = Engine::PropAccessNone{};
			break;
		case Engine::PropertyKind::Sequence:
		case Engine::PropertyKind::Associative:
			merged.meta.readOnly = true;
			merged.access = Engine::PropAccessNone{};
			break;
		case Engine::PropertyKind::Bool:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessBool, bool>(nodes, merged.meta);
			break;
		case Engine::PropertyKind::Int32:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessInt32, std::int32_t>(
			    nodes, merged.meta, AreAllEqual<std::int32_t>, BuildIntegerRangeMarker<std::int32_t>);
			break;
		case Engine::PropertyKind::Int64:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessInt64, std::int64_t>(
			    nodes, merged.meta, AreAllEqual<std::int64_t>, BuildIntegerRangeMarker<std::int64_t>);
			break;
		case Engine::PropertyKind::Float:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessFloat, float>(
			    nodes, merged.meta, AreAllFloatEqual, BuildFloatRangeMarker);
			break;
		case Engine::PropertyKind::Double:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessDouble, double>(
			    nodes, merged.meta, AreAllDoubleEqual, BuildDoubleRangeMarker);
			break;
		case Engine::PropertyKind::String:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessString, std::string>(nodes, merged.meta);
			break;
		case Engine::PropertyKind::Enum:
			merged.access = BuildMergedEnumAccess(nodes, merged.meta);
			break;
		case Engine::PropertyKind::Vec2f:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessVec2f, sf::Vector2f>(
			    nodes, merged.meta, AreAllEqual<sf::Vector2f>, BuildVec2fRangeMarker);
			break;
		case Engine::PropertyKind::Vec2i:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessVec2i, sf::Vector2i>(
			    nodes, merged.meta, AreAllEqual<sf::Vector2i>, BuildVec2iRangeMarker);
			break;
		case Engine::PropertyKind::Vec2u:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessVec2u, sf::Vector2u>(
			    nodes, merged.meta, AreAllEqual<sf::Vector2u>, BuildVec2uRangeMarker);
			break;
		case Engine::PropertyKind::Vec3f:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessVec3f, sf::Vector3f>(
			    nodes, merged.meta, AreAllEqual<sf::Vector3f>, BuildVec3fRangeMarker);
			break;
		case Engine::PropertyKind::Color:
			merged.access = BuildMergedScalarAccess<Engine::PropAccessColor, sf::Color>(nodes, merged.meta);
			break;
		}
		return merged;
	}

	Engine::PropertyTree BuildMergedTree(const std::vector<Engine::PropertyTree>& trees) {
		Engine::PropertyTree merged;
		if (trees.empty()) {
			return merged;
		}
		std::vector<const Engine::PropertyNode*> roots;
		roots.reserve(trees.size());
		for (const auto& tree : trees) {
			if (tree.roots.empty()) {
				return merged;
			}
			roots.push_back(&tree.roots.front());
		}
		if (auto mergedRoot = TryBuildMergedNode(roots)) {
			merged.roots.push_back(std::move(*mergedRoot));
		}
		return merged;
	}

	std::string BehaviourTitle(const Behaviour& behaviour) {
		return (typeid(behaviour).name() + sizeof("class"));
	}

	std::vector<std::pair<std::type_index, std::string>> FindCommonBehaviourTypes(
	    const std::vector<std::shared_ptr<SceneNode>>& nodes) {
		std::vector<std::pair<std::type_index, std::string>> common;
		if (nodes.empty()) {
			return common;
		}
		const auto& firstBehaviours = nodes.front()->GetBehaviours();
		for (const auto& behaviour : firstBehaviours) {
			if (!behaviour) {
				continue;
			}
			const Behaviour* behaviourRaw = behaviour.get();
			const auto type = std::type_index(typeid(*behaviourRaw));
			bool existsInAll = true;
			for (std::size_t i = 1; i < nodes.size(); ++i) {
				bool found = false;
				for (const auto& candidate : nodes[i]->GetBehaviours()) {
					const Behaviour* candidateRaw = candidate.get();
					if (candidateRaw && std::type_index(typeid(*candidateRaw)) == type) {
						found = true;
						break;
					}
				}
				if (!found) {
					existsInAll = false;
					break;
				}
			}
			if (!existsInAll) {
				continue;
			}
			const bool alreadyAdded = std::any_of(common.begin(), common.end(), [type](const auto& item) {
				return item.first == type;
			});
			if (!alreadyAdded) {
				common.emplace_back(type, BehaviourTitle(*behaviour));
			}
		}
		return common;
	}

	void DrawMergedProviderBlock(Engine::EditorVisualTheme::InspectorSectionHeaderStyle sectionStyle, const char* title,
	    const std::vector<Engine::IPropertiesProvider*>& inspectables, const Engine::PropertyTreeDrawer& drawer) {
		if (inspectables.empty()) {
			return;
		}
		std::vector<Engine::PropertyTree> trees;
		trees.reserve(inspectables.size());
		for (auto* inspectable : inspectables) {
			if (!inspectable) {
				return;
			}
			Engine::PropertyTree tree;
			Engine::PropertyBuilder builder(tree);
			inspectable->BuildPropertyTree(builder);
			trees.push_back(std::move(tree));
		}

		Engine::PropertyTree mergedTree = BuildMergedTree(trees);
		if (mergedTree.roots.empty()) {
			return;
		}
		Engine::EditorVisualTheme::PushInspectorSectionHeaderColors(sectionStyle);
		const bool open = ImGui::CollapsingHeader(title);
		Engine::EditorVisualTheme::PopInspectorSectionHeaderColors();
		if (!open) {
			return;
		}
		drawer.Draw(mergedTree, {.unwrapSingleRootObject = true});
	}

	void DrawIPropertiesProviderBlock(Engine::EditorVisualTheme::InspectorSectionHeaderStyle sectionStyle,
	    const char* title, Engine::IPropertiesProvider* inspectable, const std::shared_ptr<EntityOnNode>& entity,
	    Engine::EntitySlot slot, const Engine::PropertyTreeDrawer& drawer) {
		if (!inspectable) {
			return;
		}
		Engine::PropertyTree tree;
		Engine::PropertyBuilder builder(tree);
		inspectable->BuildPropertyTree(builder);
		if (tree.roots.empty() && tree.inspectorMethods.empty() && !entity) {
			return;
		}
		ImGui::PushID(static_cast<const void*>(inspectable));
		Engine::EditorVisualTheme::PushInspectorSectionHeaderColors(sectionStyle);
		const bool open = ImGui::CollapsingHeader(title);
		Engine::EditorVisualTheme::PopInspectorSectionHeaderColors();
		if (!tree.inspectorMethods.empty() || entity) {
			if (ImGui::BeginPopupContextItem("inspector_reflected_methods", ImGuiPopupFlags_MouseButtonRight)) {
				if (entity) {
					if (ImGui::MenuItem("Copy")) {
						(void)Engine::Editor::GetInstance().CopyEntity(entity, slot);
					}
					const bool canCutOrDelete = (slot != Engine::EntitySlot::Transform);
					if (!canCutOrDelete) {
						ImGui::BeginDisabled();
					}
					if (ImGui::MenuItem("Cut")) {
						(void)Engine::Editor::GetInstance().CutEntity(entity, slot);
					}
					if (ImGui::MenuItem("Delete")) {
						(void)Engine::Editor::GetInstance().DeleteEntity(entity, slot);
					}
					if (!canCutOrDelete) {
						ImGui::EndDisabled();
					}
					const bool canPaste = Engine::Editor::GetInstance().CanPasteEntityToSelectedNode();
					if (!canPaste) {
						ImGui::BeginDisabled();
					}
					if (ImGui::MenuItem("Paste")) {
						(void)Engine::Editor::GetInstance().PasteClipboard();
					}
					if (!canPaste) {
						ImGui::EndDisabled();
					}
					if (!tree.inspectorMethods.empty()) {
						ImGui::Separator();
					}
				}

				for (const auto& m : tree.inspectorMethods) {
					if (ImGui::MenuItem(fmt::format("Call {}", m.menuLabel).c_str()) && m.invoke) {
						m.invoke();
					}
				}

				ImGui::EndPopup();
			}
		}
		if (!open) {
			ImGui::PopID();
			return;
		}
		if (!tree.roots.empty()) {
			drawer.Draw(tree, {.unwrapSingleRootObject = true});
		}
		ImGui::PopID();
	}
} // namespace

namespace Engine {
	void NodeInspectorWidget::Draw(const std::vector<std::shared_ptr<SceneNode>>& nodes) const {
		std::vector<std::shared_ptr<SceneNode>> validNodes;
		validNodes.reserve(nodes.size());
		for (const auto& node : nodes) {
			if (node) {
				validNodes.push_back(node);
			}
		}
		if (validNodes.empty()) {
			ImGui::TextUnformatted("No node selected");
			return;
		}

		if (validNodes.size() == 1) {
			const auto& node = validNodes.front();
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SceneNode, "SceneNode",
			    dynamic_cast<IPropertiesProvider*>(node.get()), nullptr, EntitySlot::Behaviour, _propertyDrawer);

			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Transform, "Transform",
			    dynamic_cast<IPropertiesProvider*>(node->GetLocalTransform().get()), node->GetLocalTransform(),
			    EntitySlot::Transform, _propertyDrawer);

			if (const auto sorting = node->GetSortingStrategy()) {
				DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SortingStrategy,
				    "Sorting strategy", dynamic_cast<IPropertiesProvider*>(sorting.get()), sorting,
				    EntitySlot::SortingStrategy, _propertyDrawer);
			}
			if (const auto visual = node->GetVisual()) {
				DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Visual, "Visual",
				    dynamic_cast<IPropertiesProvider*>(visual.get()), visual, EntitySlot::Visual, _propertyDrawer);
			}
			for (const auto& behaviour : node->GetBehaviours()) {
				if (!behaviour) {
					continue;
				}
				const std::string className = BehaviourTitle(*behaviour);
				DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Behaviour,
				    className.c_str(), dynamic_cast<IPropertiesProvider*>(behaviour.get()), behaviour,
				    EntitySlot::Behaviour, _propertyDrawer);
			}
			return;
		}

		ImGui::Text("Selected nodes: %d", static_cast<int>(validNodes.size()));
		ImGui::Separator();

		std::vector<IPropertiesProvider*> sceneNodeProviders;
		std::vector<IPropertiesProvider*> transformProviders;
		sceneNodeProviders.reserve(validNodes.size());
		transformProviders.reserve(validNodes.size());
		for (const auto& node : validNodes) {
			sceneNodeProviders.push_back(dynamic_cast<IPropertiesProvider*>(node.get()));
			transformProviders.push_back(dynamic_cast<IPropertiesProvider*>(node->GetLocalTransform().get()));
		}
		DrawMergedProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SceneNode, "SceneNode",
		    sceneNodeProviders, _propertyDrawer);
		DrawMergedProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Transform, "Transform",
		    transformProviders, _propertyDrawer);

		bool hasCommonSorting = true;
		std::vector<IPropertiesProvider*> sortingProviders;
		sortingProviders.reserve(validNodes.size());
		for (const auto& node : validNodes) {
			const auto sorting = node->GetSortingStrategy();
			if (!sorting) {
				hasCommonSorting = false;
				break;
			}
			sortingProviders.push_back(dynamic_cast<IPropertiesProvider*>(sorting.get()));
		}
		if (hasCommonSorting) {
			DrawMergedProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SortingStrategy, "Sorting strategy",
			    sortingProviders, _propertyDrawer);
		}

		bool hasCommonVisual = true;
		std::vector<IPropertiesProvider*> visualProviders;
		visualProviders.reserve(validNodes.size());
		for (const auto& node : validNodes) {
			const auto visual = node->GetVisual();
			if (!visual) {
				hasCommonVisual = false;
				break;
			}
			visualProviders.push_back(dynamic_cast<IPropertiesProvider*>(visual.get()));
		}
		if (hasCommonVisual) {
			DrawMergedProviderBlock(
			    EditorVisualTheme::InspectorSectionHeaderStyle::Visual, "Visual", visualProviders, _propertyDrawer);
		}

		const auto commonBehaviours = FindCommonBehaviourTypes(validNodes);
		if (commonBehaviours.empty()) {
			ImGui::TextUnformatted("No common behaviour types across selected nodes");
			return;
		}
		for (const auto& [type, title] : commonBehaviours) {
			std::vector<IPropertiesProvider*> behaviourProviders;
			behaviourProviders.reserve(validNodes.size());
			bool foundInAll = true;
			for (const auto& node : validNodes) {
				IPropertiesProvider* provider = nullptr;
				for (const auto& behaviour : node->GetBehaviours()) {
					const Behaviour* behaviourRaw = behaviour.get();
					if (behaviourRaw && std::type_index(typeid(*behaviourRaw)) == type) {
						provider = dynamic_cast<IPropertiesProvider*>(behaviour.get());
						break;
					}
				}
				if (!provider) {
					foundInAll = false;
					break;
				}
				behaviourProviders.push_back(provider);
			}
			if (!foundInAll) {
				continue;
			}
			DrawMergedProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Behaviour, title.c_str(),
			    behaviourProviders, _propertyDrawer);
		}
	}
} // namespace Engine
