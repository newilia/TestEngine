#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>
#include <optional>
#include <string>

namespace sf {
	class RenderWindow;
}

/// Owns editor tools and routes input to the active tool before `EventsDispatcher`.
class EditorToolManager
{
public:
	EditorToolManager();
	~EditorToolManager() = default;

	EditorToolManager(const EditorToolManager&) = delete;
	EditorToolManager& operator=(const EditorToolManager&) = delete;

	int GetActiveToolIndex() const;

	void SetActiveToolIndex(int index);

	bool ProcessEvent(const sf::Event& event);
	void OnPresent(const sf::Time& dt);
	void DrawOverlay(sf::RenderWindow& window);
	void DrawActiveToolParametersUi();

	static std::optional<int> TryToolIndexFromDigitKey(sf::Keyboard::Key key);
	static std::string FormatToolPaletteLabel(int toolIndex, const char* displayName);

	/// Main keyboard row digits 1–9 then 0; applies only while index is below `kToolCount`.
	bool TryActivateToolViaDigitKey(sf::Keyboard::Key key);

	static constexpr int kToolCount = 5;

private:
	int _activeToolIndex = 0;
	std::unique_ptr<IEditorTool> _tools[kToolCount]; // TODO make dynamic
};
