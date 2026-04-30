#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>
#include <optional>
#include <string>

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

	[[nodiscard]] static std::optional<int> TryToolIndexFromDigitKey(sf::Keyboard::Key key);

	[[nodiscard]] static std::string FormatToolPaletteLabel(int toolIndex, const char* displayName);

	/// Main keyboard row digits 1–9 then 0; applies only while index is below `kToolCount`.
	[[nodiscard]] bool TryActivateToolViaDigitKey(sf::Keyboard::Key key);

private:
	int _activeToolIndex = 0;
	std::unique_ptr<IEditorTool> _tools[kToolCount];
	PullTool* _pullTool = nullptr;
};
