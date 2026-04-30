#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

class PullTool;
class VectorArrowVisual;

/// Owns editor tools and routes input to the active tool before `UserInput`.
class EditorToolManager
{
public:
	EditorToolManager();
	~EditorToolManager() = default;

	EditorToolManager(const EditorToolManager&) = delete;
	EditorToolManager& operator=(const EditorToolManager&) = delete;

	[[nodiscard]] int GetActiveToolIndex() const { return _activeToolIndex; }

	void SetActiveToolIndex(int index);

	[[nodiscard]] bool ProcessEvent(const sf::Event& event);
	void OnPresent(const sf::Time& dt);

	void BindPullArrow(std::shared_ptr<VectorArrowVisual> arrow);

	[[nodiscard]] PullTool* GetPullTool() { return _pullTool; }

	static constexpr int kToolCount = 4;

private:
	int _activeToolIndex = 0;
	std::unique_ptr<IEditorTool> _tools[kToolCount];
	PullTool* _pullTool = nullptr;
};
