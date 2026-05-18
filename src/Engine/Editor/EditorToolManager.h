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

class EditorToolManager
{
public:
	EditorToolManager();
	void Init();
	void OnUpdate(const sf::Time& dt);
	void DrawOverlay(sf::RenderWindow& window);

	int GetActiveToolIndex() const;
	void SetActiveToolIndex(int index);
	int GetToolCount() const;
	std::string GetToolFormattedName(int index) const;

	bool ProcessEvent(const sf::Event& event);
	void DrawActiveToolParametersUi();
	bool TryActivateToolViaDigitKey(sf::Keyboard::Key key);
	[[nodiscard]] std::optional<std::string> TryGetActiveToolCursorOverlayText() const;

private:
	static std::optional<int> TryToolIndexFromDigitKey(sf::Keyboard::Key key);
	static std::string FormatToolPaletteLabel(int toolIndex, const char* displayName);

private:
	int _activeToolIndex = 0;
	using ToolEntry = std::pair<std::string, std::unique_ptr<IEditorTool>>;
	std::vector<ToolEntry> _tools;
};
