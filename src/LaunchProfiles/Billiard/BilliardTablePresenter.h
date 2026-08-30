#pragma once

#include "BilliardBallBehaviour.h"
#include "BilliardPocketBehaviour.h"
#include "BilliardTableSnapshot.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <map>
#include <vector>

namespace Billiard {

	class BilliardTablePresenter
	{
	public:
		void SetBalls(std::map<int, RefWrapper<BilliardBallBehaviour>> balls);
		void SetPockets(std::vector<RefWrapper<BilliardPocketBehaviour>> pockets);
		void SetTableRect(RefWrapper<RectangleShapeVisual> tableRect);

		TableSnapshot CaptureSnapshot() const;
		void ApplySnapshot(const TableSnapshot& snapshot);
		bool AreBallsMoving() const;
		sf::FloatRect GetBallInHandRect() const;
		sf::FloatRect GetKitchenRect() const;
		float GetBallRadius() const;
		sf::Vector2f GetNearestFreeBallPosition(sf::Vector2f requestedPosition, int excludeBallNumber = -1) const;
		void OnBallPocketed(int ballNumber, shared_ptr<BilliardPocketBehaviour> pocket);
		void RestoreBall(int ballNumber);
		bool IsBallOutsideExpandedTable(int ballNumber, float margin) const;

	private:
		sf::Vector2f GetTableCenter() const;
		sf::Vector2f GetEightBallRestorePosition() const;

	private:
		std::map<int, RefWrapper<BilliardBallBehaviour>> _ballsBehaviours;
		std::vector<RefWrapper<BilliardPocketBehaviour>> _pockets;
		RefWrapper<RectangleShapeVisual> _tableRect;
	};

} // namespace Billiard
