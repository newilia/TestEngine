#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <SFML/System/Vector2.hpp>

namespace Billiard {

	class CueBallAimWidgetBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void Show();
		void Hide();
		[[nodiscard]] Signal<sf::Vector2f>& GetAimPointChangedSignal() const;
		void ResetAimPoint();
		void SetAimPoint(const sf::Vector2f& aimPoint);
		bool TrySetAimPointFromWorld(const sf::Vector2f& worldPoint);

	private:
		void UpdateAimPointPosition();

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _circleArea;
		/// @property
		RefWrapper<SceneNode> _aimPointNode;

	private:
		sf::Vector2f _aimPoint;
		mutable Signal<sf::Vector2f> _aimPointChangedSignal;
	};

} // namespace Billiard
