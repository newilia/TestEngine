#pragma once

#include "Engine/Editor/EditorHistory.h"

class SceneNode;
class Scene;

namespace Engine::EditorCommands {

	class PlaySimulationCommand final : public Engine::IEditorCommand
	{
	public:
		PlaySimulationCommand();

		bool Execute() override;
		void Undo() override;

	private:
		std::shared_ptr<Scene> _sceneSnapshot;
	};

} // namespace Engine::EditorCommands
