#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Editor/DebugSettingsWidget.h"
#include "Engine/Editor/EditorHistory.h"
#include "Engine/Editor/EditorToolManager.h"
#include "Engine/Editor/EditorToolsWidget.h"
#include "Engine/Editor/NodeInspectorWidget.h"
#include "Engine/Editor/SceneClipboard.h"
#include "Engine/Editor/SceneHierarchyWidget.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>

namespace Engine {

	class Editor : public Singleton<Editor>
	{
	public:
		void Toggle();
		void SetIsOpen(bool isOpen);
		bool IsOpen() const;
		void Update(float dt);
		void Draw();
		void OnEvent(const sf::Event& event);
		void OnResize(const sf::Event::Resized& e);
		void OnKeyPress(const sf::Event::KeyPressed& e);
		void OnKeyRelease(const sf::Event::KeyReleased& e);
		void OnMouseMove(const sf::Event::MouseMoved& e);
		void OnMouseButtonPressed(const sf::Event::MouseButtonPressed& e);
		void OnMouseButtonReleased(const sf::Event::MouseButtonReleased& e);
		void OnMouseWheelScrolled(const sf::Event::MouseWheelScrolled& e);

		std::shared_ptr<SceneNode> GetSelectedNode() const;
		void ClearNodeSelection();
		void SetSelectedNode(std::shared_ptr<SceneNode> node);
		bool CopySelectedNode();
		bool CutSelectedNode();
		bool PasteClipboard();
		bool CopyNode(const std::shared_ptr<SceneNode>& node);
		bool CutNode(const std::shared_ptr<SceneNode>& node);
		bool PasteClipboardOnto(const std::shared_ptr<SceneNode>& parent);
		bool CanPasteClipboard() const;
		bool CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool CutEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool DeleteSelectedNode();
		bool DeleteNode(const std::shared_ptr<SceneNode>& node);
		bool DeleteEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool CanPasteEntityToSelectedNode() const;

		EditorToolManager& GetEditorToolManager();
		const EditorToolManager& GetEditorToolManager() const;

	private:
		bool _isOpen = true;
		std::optional<sf::Vector2i> _cameraMoveMouseOriginPos; // nullopt when not moving
		SceneHierarchyWidget _sceneHierarchyWidget{};
		NodeInspectorWidget _nodeInspectorWidget{};
		EditorToolsWidget _editorToolsWidget{};
		DebugSettingsWidget _debugSettingsWidget{};
		std::unique_ptr<EditorToolManager> _editorToolManager{std::make_unique<EditorToolManager>()};
		EditorHistory _history{};
		SceneClipboard _clipboard{};
	};

} // namespace Engine
