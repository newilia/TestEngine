#include "PlaySimulationCommand.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNodeClone.h"

namespace Engine::EditorCommands {

	PlaySimulationCommand::PlaySimulationCommand() = default;

	bool PlaySimulationCommand::Execute() {
		auto root = MainContext::GetInstance().GetScene()->GetRoot();
		_sceneSnapshot = Engine::CloneScene(MainContext::GetInstance().GetScene());
		MainContext::GetInstance().SetSimPaused(false);
		return true;
	}

	void PlaySimulationCommand::Undo() {
		MainContext::GetInstance().SetScene(std::move(_sceneSnapshot));
		_sceneSnapshot.reset();
		MainContext::GetInstance().SetSimPaused(true);
	}

} // namespace Engine::EditorCommands
