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

		[[nodiscard]] TableSnapshot CaptureSnapshot() const;
		void ApplySnapshot(const TableSnapshot& snapshot);
		[[nodiscard]] bool AreBallsMoving() const;
		[[nodiscard]] sf::FloatRect GetBallInHandRect() const;
		[[nodiscard]] sf::FloatRect GetKitchenRect() const;
		[[nodiscard]] sf::Vector2f GetTableCenter() const;
		[[nodiscard]] float GetBallRadius() const;
		void RestoreBall(int ballNumber);
		bool IsBallOutsideExpandedTable(int ballNumber, float margin) const;

	private:
		std::map<int, RefWrapper<BilliardBallBehaviour>> _ballsBehaviours;
		std::vector<RefWrapper<BilliardPocketBehaviour>> _pockets;
		RefWrapper<RectangleShapeVisual> _tableRect;
	};

} // namespace Billiard
