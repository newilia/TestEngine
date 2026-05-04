#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <cstddef>
#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	class CutNodeCommand final : public Engine::IEditorCommand
	{
	public:
		explicit CutNodeCommand(std::shared_ptr<SceneNode> node);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		std::weak_ptr<SceneNode> _parent;
		std::size_t _index = 0;
	};

} // namespace Engine::EditorCommands
