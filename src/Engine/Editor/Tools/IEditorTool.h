#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace sf {
	class RenderWindow;
}

/// Active editor tool: scene input is routed here before `EventsDispatcher` when ImGui does not capture.
class IEditorTool
{
public:
	virtual ~IEditorTool() = default;

	/// Return true if the tool handled the event and it must not propagate to `EventsDispatcher` / scene handlers.
	virtual bool processEvent(const sf::Event& event) = 0;

	/// Optional per-frame hook (e.g. Pull force arrow sync); called during present after scene `NotifyPresentRec`.
	virtual void onPresent(const sf::Time& /*dt*/) {}

	/// Optional world overlay after the scene is drawn (same frame as ImGui `Update`…`Render`).
	virtual void drawOverlay(sf::RenderWindow& window);

	/// Optional ImGui block in the editor tools "Parameters" section while this tool is active.
	virtual void drawToolParametersUi() {}
};
