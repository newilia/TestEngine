#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	class PasteNodeCommand final : public Engine::IEditorCommand
	{
	public:
		PasteNodeCommand(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> node);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _parent;
		std::shared_ptr<SceneNode> _node;
	};

} // namespace Engine::EditorCommands
