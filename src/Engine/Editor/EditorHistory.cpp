#include "Engine/Editor/EditorHistory.h"

namespace Engine {

	bool EditorHistory::Execute(std::unique_ptr<IEditorCommand> command) {
		if (!command || !command->Execute()) {
			return false;
		}
		_redoStack.clear();
		_undoStack.push_back(std::move(command));
		return true;
	}

	bool EditorHistory::Undo() {
		if (_undoStack.empty()) {
			return false;
		}
		auto command = std::move(_undoStack.back());
		_undoStack.pop_back();
		command->Undo();
		_redoStack.push_back(std::move(command));
		return true;
	}

	bool EditorHistory::Redo() {
		if (_redoStack.empty()) {
			return false;
		}
		auto command = std::move(_redoStack.back());
		_redoStack.pop_back();
		if (!command->Execute()) {
			return false;
		}
		_undoStack.push_back(std::move(command));
		return true;
	}

} // namespace Engine
