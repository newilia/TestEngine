#pragma once

#include "Engine/Core/Singleton.h"
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
		[[nodiscard]] std::shared_ptr<SceneNode> GetSelectedNode() const;
		void ClearNodeSelection();

	private:
		bool _isOpen = true;
		SceneHierarchyWidget _sceneHierarchyWidget;
		NodeInspectorWidget _nodeInspectorWidget;
	};

} // namespace Engine
