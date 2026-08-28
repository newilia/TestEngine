#pragma once

#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/TextVisual.h"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <cstddef>
#include <string>

/// In-scene single-line text field backed by a `TextVisual`. Tap the visual to edit; Esc, Enter, or tap
/// outside ends editing. Arrow keys move a `|` cursor; typed text is emitted via `OnTextChanged`.
class InputTextBehaviour : public EventHandlerBehaviourBase
{
	META_CLASS()

public:
	void OnInit() override;
	void OnDeinit() override;
	void OnUpdate(const sf::Time& dt) override;
	void OnEvent(const sf::Event& event) override;

	[[nodiscard]] std::string GetCurrentText() const;
	Signal<std::string>& GetOnTextChangedSignal() const;

private:
	void StartEditing();
	void StopEditing();
	void ResetCursorBlink();
	void RefreshVisual();
	void EmitTextChanged();
	void InsertCharacter(char32_t codePoint);
	void DeleteBeforeCursor();
	void MoveCursorLeft();
	void MoveCursorRight();
	bool HitTestTextVisual(const sf::Vector2f& worldPoint) const;
	void OnPointerDown(const sf::Vector2f& worldPoint);

	/// @property
	RefWrapper<TextVisual> _textVisual;

	mutable Signal<std::string> _onTextChanged;

	std::string _text;
	std::size_t _cursorPos = 0;
	bool _isEditing = false;
	bool _isCursorVisible = true;
	sf::Time _cursorBlinkElapsed{};
};
