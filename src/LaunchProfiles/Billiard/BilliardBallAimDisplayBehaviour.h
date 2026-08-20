#pragma once

#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Billiard {

	class BilliardBallAimDisplayBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()

	public:
		void SetInputEnabled(bool enabled);
		[[nodiscard]] bool IsInputEnabled() const;
		void OnEvent(const sf::Event& event) override;

	public:
		void Show();
		void Hide();
		[[nodiscard]] Signal<sf::Vector2f>& GetAimPointChangedSignal();
		void ResetAimPoint();
		void SetAimPoint(const sf::Vector2f& aimPoint);

	private:
		void UpdateAimPointPosition();
		void OnTap(const sf::Vector2f& worldPoint);

	private:
		/// @property
		RefWrapper<CircleShapeVisual> _circleArea;
		/// @property
		RefWrapper<SceneNode> _aimPointNode;

	private:
		bool _inputEnabled = false;
		bool _isPointerDown = false;
		sf::Vector2f _aimPoint;
		Signal<sf::Vector2f> _aimPointChangedSignal;
	};

} // namespace Billiard
