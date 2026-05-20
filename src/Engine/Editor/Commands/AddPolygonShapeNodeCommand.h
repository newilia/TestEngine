#pragma once

#include "Engine/Core/ColorUtils.h"
#include "Engine/Editor/EditorHistory.h"

#include <SFML/System/Vector2.hpp>

#include <cstddef>
#include <memory>
#include <vector>

class SceneNode;

namespace Engine::EditorCommands {

	class AddPolygonShapeNodeCommand final : public Engine::IEditorCommand
	{
	public:
		AddPolygonShapeNodeCommand(std::shared_ptr<SceneNode> parent, std::size_t index, sf::Vector2f centerWorld,
		    std::vector<sf::Vector2f> localPoints, bool attachPhysics, Utils::HsvShapeColors colors);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _parent;
		std::size_t _index = 0;
		std::weak_ptr<SceneNode> _node;
		sf::Vector2f _centerWorld{};
		std::vector<sf::Vector2f> _localPoints;
		bool _attachPhysics = false;
		Utils::HsvShapeColors _colors{};
	};

} // namespace Engine::EditorCommands
