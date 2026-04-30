#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

/// Active editor tool: scene input is routed here before `UserInput` when ImGui does not capture.
class IEditorTool
{
public:
	virtual ~IEditorTool() = default;

	/// Return true if the tool handled the event and it must not propagate to `UserInput` / scene handlers.
	[[nodiscard]] virtual bool processEvent(const sf::Event& event) = 0;

	/// Optional per-frame hook (e.g. Pull force arrow sync); called during present after scene `NotifyPresentRec`.
	virtual void onPresent(const sf::Time& /*dt*/) {}
};
