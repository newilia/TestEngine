#pragma once

#include "Engine/Core/ColorUtils.h"
#include "Engine/Editor/EditorHistory.h"

#include <SFML/System/Vector2.hpp>

#include <cstddef>
#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	class AddRectangleShapeNodeCommand final : public Engine::IEditorCommand
	{
	public:
		AddRectangleShapeNodeCommand(std::shared_ptr<SceneNode> parent, std::size_t index, sf::Vector2f centerWorld,
		    sf::Vector2f size, bool attachPhysics, Utils::HsvShapeColors colors);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _parent;
		std::size_t _index = 0;
		std::weak_ptr<SceneNode> _node;
		sf::Vector2f _centerWorld{};
		sf::Vector2f _size{};
		bool _attachPhysics = false;
		Utils::HsvShapeColors _colors{};
	};

} // namespace Engine::EditorCommands
