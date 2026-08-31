#include "LocalHumanPlayer.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

namespace Billiard {

	LocalHumanPlayer::LocalHumanPlayer(int playerIndex, BallInHandInputController* ballInHandInput,
	    CueInputController* cueInput, CueBallAimInputController* cueBallAimInput)
	    : _playerIndex(playerIndex), _ballInHandInput(ballInHandInput), _cueInput(cueInput),
	      _cueBallAimInput(cueBallAimInput) {}

	void LocalHumanPlayer::OnTurnStarted(const TableSnapshot& /*table*/, const RulesSnapshot& /*rules*/) {
		_turnId++;
		_inputEnabled = true;

		if (_ballInHandInput) {
			_ballInHandInput->SetInputEnabled(true);
		}
		if (_cueInput) {
			_cueInput->SetInputEnabled(true);
		}
	}

	void LocalHumanPlayer::OnTurnUpdate(const sf::Time& /*deltaTime*/) {}

	void LocalHumanPlayer::OnTurnEnded() {
		_inputEnabled = false;
		if (_ballInHandInput) {
			_ballInHandInput->SetInputEnabled(false);
		}
		if (_cueInput) {
			_cueInput->SetInputEnabled(false);
		}
	}

	void LocalHumanPlayer::OnEvent(const sf::Event& event) {
		if (!_inputEnabled) {
			return;
		}

		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (_ballInHandInput && _ballInHandInput->IsInputEnabled()) {
				_ballInHandInput->OnMouseButtonPressed(pressed->position, pressed->button);
			}
			if (_cueBallAimInput && _cueBallAimInput->IsInputEnabled()) {
				_cueBallAimInput->OnMouseButtonPressed(pressed->position, pressed->button);
			}
			if (_cueInput && _cueInput->IsInputEnabled()) {
				_cueInput->OnMouseButtonPressed(pressed->position, pressed->button);
			}
			return;
		}

		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			if (_ballInHandInput && _ballInHandInput->IsInputEnabled()) {
				_ballInHandInput->OnMouseMoved(moved->position);
			}
			if (_cueBallAimInput && _cueBallAimInput->IsInputEnabled()) {
				_cueBallAimInput->OnMouseMoved(moved->position);
			}
			if (_cueInput && _cueInput->IsInputEnabled()) {
				_cueInput->OnMouseMoved(moved->position);
			}
			return;
		}

		if (const auto* scrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
			if (_cueInput && _cueInput->IsInputEnabled() && scrolled->wheel == sf::Mouse::Wheel::Vertical) {
				_cueInput->OnMouseWheelScrolled(scrolled->delta);
			}
			return;
		}

		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (_ballInHandInput && _ballInHandInput->IsInputEnabled()) {
				_ballInHandInput->OnMouseButtonReleased(released->position);
			}
			if (_cueBallAimInput && _cueBallAimInput->IsInputEnabled()) {
				_cueBallAimInput->OnMouseButtonReleased(released->position, released->button);
			}
			if (_cueInput && _cueInput->IsInputEnabled()) {
				_cueInput->OnMouseButtonReleased(released->position, released->button, _playerIndex, _turnId);
			}
		}
	}

	bool LocalHumanPlayer::HasPendingIntent() const {
		return _cueInput && _cueInput->HasPendingIntent();
	}

	std::optional<TurnIntent> LocalHumanPlayer::ConsumeIntent() {
		if (_cueInput) {
			return _cueInput->ConsumePendingIntent();
		}
		return std::nullopt;
	}

	bool LocalHumanPlayer::WantsInput() const {
		return _inputEnabled;
	}

} // namespace Billiard
