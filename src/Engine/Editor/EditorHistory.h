#pragma once

#include <memory>
#include <vector>

namespace Engine {

	class IEditorCommand
	{
	public:
		virtual ~IEditorCommand() = default;
		virtual bool Execute() = 0;
		virtual void Undo() = 0;
	};

	class EditorHistory
	{
	public:
		bool Execute(std::unique_ptr<IEditorCommand> command);
		bool Undo();
		bool Redo();

	private:
		std::vector<std::unique_ptr<IEditorCommand>> _undoStack;
		std::vector<std::unique_ptr<IEditorCommand>> _redoStack;
	};

} // namespace Engine
