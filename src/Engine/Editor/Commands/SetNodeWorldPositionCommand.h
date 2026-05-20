#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	class SetNodeWorldPositionCommand final : public Engine::IEditorCommand
	{
	public:
		SetNodeWorldPositionCommand(
		    std::shared_ptr<SceneNode> node, sf::Vector2f oldWorldPos, sf::Vector2f newWorldPos);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		sf::Vector2f _oldWorldPos{};
		sf::Vector2f _newWorldPos{};
	};

} // namespace Engine::EditorCommands
