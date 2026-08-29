#pragma once

#include "BallType.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/AssetRef.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneObject.h"
#include "Engine/Visual/TextVisual.h"

#include <string>

namespace Billiard {

	enum class FoulKind;

	class BilliardScoreboardBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void Reset();
		void SetActivePlayerIndex(int playerIndex);
		void ShowMessage(const std::string& message);
		void ShowFoulMessage(FoulKind foulKind);
		void SetRemainingTurnTime(float seconds);
		void SetPlayerBallType(int playerIndex, BallType ballType);
		void SetPlayerName(int playerIndex, const std::string& name);
		void OnBallPocketed(int ballNumber);

	private:
		void CreatePocketedBalls(int playerIndex);
		void AddPocketedBall(shared_ptr<SceneNode> parentNode, int ballNumber);

	private:
		std::set<int> _pocketedBalls;
		std::array<BallType, 2> _playerBallTypes;

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
		/// @property
		AssetRef<SceneObject> _pocketedBallAsset;
		/// @property
		RefWrapper<SceneNode> _player1pocketedBallsParent;
		/// @property
		RefWrapper<SceneNode> _player2pocketedBallsParent;
		/// @property
		int _pocketdBallWidth = 50;
		/// @property(tooltip="fmt placeholder for ball id")
		std::string _pocketedBallTexturePathMask = "resources/textures/8ball/ball_{}.png";
	};

} // namespace Billiard
