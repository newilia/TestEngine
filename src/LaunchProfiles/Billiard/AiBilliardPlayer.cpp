#include "AiBilliardPlayer.h"

#include <cmath>
#include <random>

namespace Billiard {

	namespace {
		std::mt19937& GetRng() {
			static std::mt19937 rng{std::random_device{}()};
			return rng;
		}
	} // namespace

	AiBilliardPlayer::AiBilliardPlayer(int playerIndex) : _playerIndex(playerIndex) {}

	void AiBilliardPlayer::OnTurnStarted(const TableSnapshot& /*table*/, const RulesSnapshot& /*rules*/) {
		_turnId++;
		_pendingIntent.reset();
		_hasShotThisTurn = false;
		_thinkTimeRemaining = 0.75f;
	}

	void AiBilliardPlayer::OnTurnUpdate(const sf::Time& deltaTime) {
		if (_hasShotThisTurn || _pendingIntent.has_value()) {
			return;
		}
		_thinkTimeRemaining -= deltaTime.asSeconds();
		if (_thinkTimeRemaining > 0.f) {
			return;
		}

		std::uniform_real_distribution<float> angleDist(0.f, 6.2831853f);
		std::uniform_real_distribution<float> pullDist(120.f, 350.f);
		std::uniform_real_distribution<float> spinDist(-0.5f, 0.5f);

		TurnIntent intent;
		intent.phase = TurnIntentPhase::Shoot;
		intent.playerIndex = _playerIndex;
		intent.turnId = _turnId;
		intent.directionAngle = sf::radians(angleDist(GetRng()));
		intent.pullDistance = pullDist(GetRng());
		intent.lateralSpin = spinDist(GetRng());
		intent.verticalSpin = spinDist(GetRng());
		_pendingIntent = intent;
		_hasShotThisTurn = true;
	}

	void AiBilliardPlayer::OnTurnEnded() {
		_pendingIntent.reset();
		_hasShotThisTurn = false;
	}

	void AiBilliardPlayer::OnEvent(const sf::Event& /*event*/) {}

	bool AiBilliardPlayer::HasPendingIntent() const {
		return _pendingIntent.has_value();
	}

	std::optional<TurnIntent> AiBilliardPlayer::ConsumeIntent() {
		auto intent = _pendingIntent;
		_pendingIntent.reset();
		return intent;
	}

	bool AiBilliardPlayer::WantsInput() const {
		return false;
	}

} // namespace Billiard
