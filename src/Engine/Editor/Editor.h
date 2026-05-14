#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Editor/DebugSettingsWidget.h"
#include "Engine/Editor/EditorHistory.h"
#include "Engine/Editor/EditorToolManager.h"
#include "Engine/Editor/EditorToolsWidget.h"
#include "Engine/Editor/GameBackgroundWidget.h"
#include "Engine/Editor/NodeInspectorWidget.h"
#include "Engine/Editor/PhysicsVisualizer.h"
#include "Engine/Editor/SceneClipboard.h"
#include "Engine/Editor/SceneHierarchyWidget.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <imgui.h>

#include <memory>
#include <unordered_set>
#include <vector>

namespace sf {
	class RenderWindow;
}

namespace Engine {

	class Editor : public Singleton<Editor>
	{
		friend class Singleton<Editor>;

	public:
		void Toggle();
		void SetIsOpen(bool isOpen);
		bool IsOpen() const;
		void OnUpdate(const sf::Time dt);
		void Draw(sf::RenderWindow& window);
		void OnEvent(const sf::Event& event);
		void OnResize(const sf::Event::Resized& e);
		void OnKeyPress(const sf::Event::KeyPressed& e);
		void OnKeyRelease(const sf::Event::KeyReleased& e);
		void OnMouseMove(const sf::Event::MouseMoved& e);
		void OnMouseButtonPressed(const sf::Event::MouseButtonPressed& e);
		void OnMouseButtonReleased(const sf::Event::MouseButtonReleased& e);
		void OnMouseWheelScrolled(const sf::Event::MouseWheelScrolled& e);

		std::shared_ptr<SceneNode> GetSelectedNode() const;
		std::vector<std::shared_ptr<SceneNode>> GetSelectedNodes() const;
		bool IsNodeSelected(const SceneNode& node) const;
		void ClearNodeSelection();
		void SetSelectedNode(std::shared_ptr<SceneNode> node);
		void ToggleSelectedNode(std::shared_ptr<SceneNode> node);
		void AddSelectedNodes(const std::vector<std::shared_ptr<SceneNode>>& nodes);
		void SetSelectedNodes(std::vector<std::shared_ptr<SceneNode>> nodes);
		bool CopySelectedNode();
		bool CutSelectedNode();
		bool PasteClipboard();
		bool CopyNode(const std::shared_ptr<SceneNode>& node);
		bool CutNode(const std::shared_ptr<SceneNode>& node);
		bool PasteClipboardOnto(const std::shared_ptr<SceneNode>& parent);
		bool CanPasteClipboard() const;
		bool CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool CutEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool DeleteSelectedNodes();
		bool DeleteNode(const std::shared_ptr<SceneNode>& node);
		bool DeleteEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool CanPasteEntityToSelectedNode() const;

		EditorToolManager& GetEditorToolManager();
		const EditorToolManager& GetEditorToolManager() const;

		PhysicsVisualizer& GetPhysicsVisualizer();
		const PhysicsVisualizer& GetPhysicsVisualizer() const;

	private:
		Editor();

		void DrawLayout();
		bool SaveSceneToPrefab();
		bool LoadSceneFromPrefab();
		void TryApplyDefaultEditorDockLayout(ImGuiID dockspaceId, const ImVec2& dockspaceSize) const;
		void DrawCursorWorldCoordsOverlay(sf::RenderWindow& window);
		void DrawViewportSelectionOverlay(sf::RenderWindow& window);

		static bool IsNodeInSubtree(
		    const std::shared_ptr<SceneNode>& candidate, const std::shared_ptr<SceneNode>& treeRoot);
		static std::optional<sf::FloatRect> TryGetHierarchySelectionBounds(const SceneNode& node);
		static void AppendHierarchyAabbOutlineLines(
		    sf::VertexArray& lines, const sf::FloatRect& bounds, const sf::Color& outlineColor, float padPx);
		static void CollectHierarchyFallbackMarker(
		    const SceneNode& node, const sf::Color& outlineColor, std::vector<sf::CircleShape>& outCircles);
		static void GatherDescendantHierarchySelectionOutlines(const SceneNode& parent,
		    const std::unordered_set<const SceneNode*>& selectedSet, sf::VertexArray& lineOutlines,
		    std::vector<sf::CircleShape>& fallbackMarkers);
		static void GatherPrimaryHierarchySelectionOutline(
		    const SceneNode& node, sf::VertexArray& lineOutlines, std::vector<sf::CircleShape>& fallbackMarkers);

		bool _isOpen = true;
		mutable bool _isLayoutFinished = false;
		std::optional<sf::Vector2i> _cameraMoveMouseOriginPos; // nullopt when not moving
		SceneHierarchyWidget _sceneHierarchyWidget{};
		NodeInspectorWidget _nodeInspectorWidget{};
		EditorToolsWidget _editorToolsWidget{};
		DebugSettingsWidget _debugSettingsWidget{};
		GameBackgroundWidget _gameBackgroundWidget{};
		std::unique_ptr<EditorToolManager> _editorToolManager{std::make_unique<EditorToolManager>()};
		EditorHistory _history{};
		SceneClipboard _clipboard{};
		PhysicsVisualizer _physicsVisualizer{};
	};

} // namespace Engine
