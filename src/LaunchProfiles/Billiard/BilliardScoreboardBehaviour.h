#pragma once

#include "BallType.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/TextVisual.h"

#include <string>

namespace Billiard {

	class BilliardScoreboardBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void Reset();
		void SetActivePlayerIndex(int playerIndex);
		void ShowMessage(const std::string& message);
		void SetRemainingTurnTime(float seconds);
		void SetPlayerBallType(int playerIndex, BallType ballType);
		void SetPlayerName(int playerIndex, const std::string& name);

	private:
		/// @property
		RefWrapper<SceneNode> _player1activeNodeRef;
		/// @property
		RefWrapper<SceneNode> _player2activeNodeRef;
		/// @property
		RefWrapper<TextVisual> _messageTextRef;
		/// @property
		RefWrapper<TextVisual> _timerTextRef;
		/// @property
		RefWrapper<TextVisual> _player1ballTypeTextRef;
		/// @property
		RefWrapper<TextVisual> _player2ballTypeTextRef;
		/// @property
		RefWrapper<TextVisual> _player1nameTextRef;
		/// @property
		RefWrapper<TextVisual> _player2nameTextRef;
	};

} // namespace Billiard
