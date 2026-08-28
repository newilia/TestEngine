#pragma once

#include "BilliardBallBehaviour.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/System/Angle.hpp>

#include <vector>

namespace Billiard {

	class AimGuideLineBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void SetBalls(std::vector<std::weak_ptr<BilliardBallBehaviour>> balls);
		void SetCueBallIndex(int cueBallIndex);
		void SetDirectionAngle(sf::Angle directionAngle);
		void Recalculate();

		void Show();
		void Hide();

	private:
		void HideGuideVisuals();
		void PlaceRay(const std::shared_ptr<RectangleShapeVisual>& visual, const sf::Vector2f& worldStart,
		    const sf::Vector2f& worldDir, float length) const;

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _imaginaryBall;
		/// @property
		RefWrapper<RectangleShapeVisual> _ray1;
		/// @property
		RefWrapper<RectangleShapeVisual> _ray2;
		/// @property
		RefWrapper<RectangleShapeVisual> _ray3;
		/// @property
		std::vector<RefWrapper<RectangleShapeVisual>> _tableRails;
		/// @property(minValue=0)
		int _maxRayLength = 100;

	private:
		std::vector<std::weak_ptr<BilliardBallBehaviour>> _balls;
		int _cueBallIndex = 0;
		sf::Angle _directionAngle{};
		bool _isGuideVisible = true;
	};

} // namespace Billiard
