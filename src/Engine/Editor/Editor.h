#pragma once

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Editor/DebugSettingsWidget.h"
#include "Engine/Editor/EditorToolManager.h"
#include "Engine/Editor/EditorToolsWidget.h"
#include "Engine/Editor/NodeInspectorWidget.h"
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
		void OnResize(const sf::Vector2u& size);
		void OnKeyPress(const sf::Keyboard::Key& key);
		void OnKeyRelease(const sf::Keyboard::Key& key);
		void OnMouseMove(const sf::Vector2i& position);
		void OnMouseButtonPressed(const sf::Event::MouseButtonPressed& e);
		void OnMouseButtonReleased(const sf::Event::MouseButtonReleased& e);

		std::shared_ptr<SceneNode> GetSelectedNode() const;
		void ClearNodeSelection();
		void SetSelectedNode(std::shared_ptr<SceneNode> node);

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
	};

} // namespace Engine
