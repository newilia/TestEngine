#include "LocalHumanPlayer.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

namespace Billiard {

	LocalHumanPlayer::LocalHumanPlayer(
	    int playerIndex, std::weak_ptr<BilliardCueBehaviour> cue, std::weak_ptr<BilliardBallBehaviour> cueBall)
	    : _playerIndex(playerIndex), _cue(std::move(cue)), _cueBall(std::move(cueBall)) {}

	void LocalHumanPlayer::OnTurnStarted(const TableSnapshot& /*table*/, const RulesSnapshot& rules) {
		_turnId++;
		_pendingIntent.reset();
		_inputEnabled = true;

		if (auto cue = _cue.lock()) {
			cue->SetInputEnabled(true);
		}
		if (auto cueBall = _cueBall.lock()) {
			cueBall->SetInputEnabled(cueBall->IsBallInHand());
		}
	}

	void LocalHumanPlayer::OnTurnUpdate(const sf::Time& /*deltaTime*/) {}

	void LocalHumanPlayer::OnTurnEnded() {
		_inputEnabled = false;
		if (auto cue = _cue.lock()) {
			cue->SetInputEnabled(false);
		}
		if (auto cueBall = _cueBall.lock()) {
			cueBall->SetInputEnabled(false);
		}
	}

	void LocalHumanPlayer::OnEvent(const sf::Event& event) {
		if (!_inputEnabled) {
			return;
		}

		auto cue = _cue.lock();
		auto cueBall = _cueBall.lock();
		if (!cue) {
			return;
		}

		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (cueBall && cueBall->IsBallInHand()) {
				cueBall->HandleMouseButtonPressed(pressed->position, pressed->button);
			}
			if (pressed->button == sf::Mouse::Button::Left) {
				if (cue->CanInteract() && cue->HitTestWorld(MapPixelToWorld(pressed->position))) {
					cue->BeginAiming();
				}
			}
			else if (pressed->button == sf::Mouse::Button::Right) {
				if (cue->CanInteract() && cue->HitTestWorld(MapPixelToWorld(pressed->position))) {
					cue->BeginPullBack(MapPixelToWorld(pressed->position));
				}
			}
			return;
		}

		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			const sf::Vector2f worldPoint = MapPixelToWorld(moved->position);
			if (cueBall && cueBall->IsBallInHand()) {
				cueBall->HandleMouseMoved(moved->position);
			}
			cue->ProcessPointerMove(worldPoint);
			return;
		}

		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (cueBall && cueBall->IsBallInHand()) {
				cueBall->HandleMouseButtonReleased(released->position);
			}
			if (released->button == sf::Mouse::Button::Left) {
				cue->StopAiming();
			}
			else if (released->button == sf::Mouse::Button::Right) {
				if (cue->CanInteract()) {
					_pendingIntent = cue->BuildTurnIntent(_playerIndex, _turnId);
				}
				cue->TryReleaseShot();
			}
		}
	}

	bool LocalHumanPlayer::HasPendingIntent() const {
		return _pendingIntent.has_value();
	}

	std::optional<TurnIntent> LocalHumanPlayer::ConsumeIntent() {
		auto intent = _pendingIntent;
		_pendingIntent.reset();
		return intent;
	}

	bool LocalHumanPlayer::WantsInput() const {
		return _inputEnabled;
	}

	sf::Vector2f LocalHumanPlayer::MapPixelToWorld(sf::Vector2i pixel) const {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return {};
		}
		return Utils::MapWindowPixelToWorld(*window, pixel);
	}

} // namespace Billiard
