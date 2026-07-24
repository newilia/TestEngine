#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/System/Time.hpp>

#include <string>

namespace Billiard {

	class BilliardScoreboardBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

		void SetPlayerScore(int playerIndex, int score);
		void AddPlayerScore(int playerIndex, int delta);
		void ShowMessage(const std::string& message);
		void SetTimer(float seconds);
		void SetActivePlayer(int playerIndex);

	private:
		void UpdateTimer(const sf::Time& dt);

		/// @property
		RefWrapper<SceneNode> _scorePanelRef;
		/// @property
		RefWrapper<SceneNode> _messagePanelRef;
		/// @property
		RefWrapper<SceneNode> _timerPanelRef;

		int _player1Score = 0;
		int _player2Score = 0;
		int _activePlayerIndex = 0;
		std::string _message;
		float _timerSeconds = 0.f;
	};

} // namespace Billiard
