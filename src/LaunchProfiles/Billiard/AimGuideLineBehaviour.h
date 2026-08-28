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

		void OnUpdate(const sf::Time& dt) override;

	private:
		void ApplySecondaryRayLengths() const;
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
		int _maxRayLength = 300;
		/// @property(minValue=0)
		int _minRayLength = 100;
		/// @property(minValue=0)
		float _rayLengthIncreaseSpeed = 10;
		/// @property(minValue=0)
		int _rayLengthDecreaseSpeed = 10;

	private:
		std::vector<std::weak_ptr<BilliardBallBehaviour>> _balls;
		int _cueBallIndex = 0;
		sf::Angle _directionAngle{};
		bool _isGuideVisible = true;
		float _secondaryRayLength = 100.f;
		float _ray2LengthFactor = 0.f;
		float _ray3LengthFactor = 0.f;
	};

} // namespace Billiard
