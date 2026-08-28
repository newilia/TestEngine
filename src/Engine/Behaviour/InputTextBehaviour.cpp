#include "InputTextBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "InputTextBehaviour.generated.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <string>

namespace {

	[[nodiscard]] std::string CodePointToUtf8(char32_t codePoint) {
		std::string result;
		if (codePoint <= 0x7F) {
			result.push_back(static_cast<char>(codePoint));
		}
		else if (codePoint <= 0x7FF) {
			result.push_back(static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F)));
			result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
		}
		else if (codePoint <= 0xFFFF) {
			result.push_back(static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F)));
			result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
		}
		else if (codePoint <= 0x10FFFF) {
			result.push_back(static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07)));
			result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
			result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
		}
		return result;
	}

	[[nodiscard]] std::size_t Utf8CharStartBefore(const std::string& text, std::size_t cursorPos) {
		if (cursorPos == 0) {
			return 0;
		}

		std::size_t index = cursorPos - 1;
		while (index > 0 && (static_cast<unsigned char>(text[index]) & 0xC0) == 0x80) {
			--index;
		}
		return index;
	}

} // namespace

void InputTextBehaviour::OnInit() {
	EventHandlerBehaviourBase::OnInit();

	if (auto textVisual = _textVisual.Get()) {
		_text = textVisual->GetString();
		_cursorPos = _text.size();
		RefreshVisual();
	}
}

void InputTextBehaviour::OnDeinit() {
	if (_isEditing) {
		StopEditing();
	}
	EventHandlerBehaviourBase::OnDeinit();
}

void InputTextBehaviour::OnUpdate(const sf::Time& dt) {
	if (!_isEditing) {
		return;
	}

	_cursorBlinkElapsed += dt;
	constexpr sf::Time blinkHalfPeriod = sf::milliseconds(500);
	while (_cursorBlinkElapsed >= blinkHalfPeriod) {
		_cursorBlinkElapsed -= blinkHalfPeriod;
		_isCursorVisible = !_isCursorVisible;
		RefreshVisual();
	}
}

void InputTextBehaviour::OnEvent(const sf::Event& event) {
	auto window = Engine::MainContext::GetInstance().GetMainWindow();
	if (!window) {
		return;
	}

	const auto toWorld = [&](sf::Vector2i pixel) {
		return Utils::MapWindowPixelToWorld(*window, pixel);
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			OnPointerDown(toWorld(pressed->position));
		}
		return;
	}

	if (const auto* began = event.getIf<sf::Event::TouchBegan>()) {
		if (began->finger == 0) {
			OnPointerDown(toWorld(began->position));
		}
		return;
	}

	if (!_isEditing) {
		return;
	}

	if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
		switch (keyPressed->code) {
		case sf::Keyboard::Key::Escape:
		case sf::Keyboard::Key::Enter:
			StopEditing();
			return;
		case sf::Keyboard::Key::Left:
			MoveCursorLeft();
			return;
		case sf::Keyboard::Key::Right:
			MoveCursorRight();
			return;
		case sf::Keyboard::Key::Backspace:
			DeleteBeforeCursor();
			return;
		default:
			return;
		}
	}

	if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
		InsertCharacter(static_cast<char32_t>(textEntered->unicode));
	}
}

std::string InputTextBehaviour::GetCurrentText() const {
	return _text;
}

Signal<std::string>& InputTextBehaviour::GetOnTextChangedSignal() const {
	return _onTextChanged;
}

void InputTextBehaviour::StartEditing() {
	_isEditing = true;
	_cursorPos = _text.size();
	ResetCursorBlink();
	RefreshVisual();
}

void InputTextBehaviour::ResetCursorBlink() {
	_isCursorVisible = true;
	_cursorBlinkElapsed = sf::Time::Zero;
}

void InputTextBehaviour::StopEditing() {
	_isEditing = false;
	RefreshVisual();
}

void InputTextBehaviour::RefreshVisual() {
	auto textVisual = _textVisual.Get();
	if (!textVisual) {
		return;
	}

	Verify(_cursorPos <= _text.size());

	if (!_isEditing) {
		textVisual->SetString(_text);
		return;
	}

	std::string display = _text;
	if (_isCursorVisible) {
		display.insert(_cursorPos, "|");
	}
	textVisual->SetString(display);
}

void InputTextBehaviour::EmitTextChanged() {
	_onTextChanged.Emit(_text);
}

void InputTextBehaviour::InsertCharacter(char32_t codePoint) {
	if (codePoint < 0x20 || codePoint == 0x7F) {
		return;
	}

	const std::string utf8 = CodePointToUtf8(codePoint);
	if (utf8.empty()) {
		return;
	}

	_text.insert(_cursorPos, utf8);
	_cursorPos += utf8.size();
	ResetCursorBlink();
	RefreshVisual();
	EmitTextChanged();
}

void InputTextBehaviour::DeleteBeforeCursor() {
	if (_cursorPos == 0) {
		return;
	}

	const std::size_t charStart = Utf8CharStartBefore(_text, _cursorPos);
	_text.erase(charStart, _cursorPos - charStart);
	_cursorPos = charStart;
	ResetCursorBlink();
	RefreshVisual();
	EmitTextChanged();
}

void InputTextBehaviour::MoveCursorLeft() {
	if (_cursorPos == 0) {
		return;
	}

	_cursorPos = Utf8CharStartBefore(_text, _cursorPos);
	ResetCursorBlink();
	RefreshVisual();
}

void InputTextBehaviour::MoveCursorRight() {
	if (_cursorPos >= _text.size()) {
		return;
	}

	std::size_t nextPos = _cursorPos + 1;
	while (nextPos < _text.size() && (static_cast<unsigned char>(_text[nextPos]) & 0xC0) == 0x80) {
		++nextPos;
	}
	_cursorPos = nextPos;
	ResetCursorBlink();
	RefreshVisual();
}

bool InputTextBehaviour::HitTestTextVisual(const sf::Vector2f& worldPoint) const {
	if (auto textVisual = _textVisual.Get()) {
		return textVisual->HitTest(worldPoint);
	}
	return false;
}

void InputTextBehaviour::OnPointerDown(const sf::Vector2f& worldPoint) {
	const bool hit = HitTestTextVisual(worldPoint);
	if (_isEditing) {
		if (!hit) {
			StopEditing();
		}
		return;
	}

	if (hit) {
		StartEditing();
	}
}
