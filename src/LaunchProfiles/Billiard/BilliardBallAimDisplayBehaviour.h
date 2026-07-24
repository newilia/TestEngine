#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Billiard {

	class BilliardBallAimDisplayBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& dt) override;

		void Show();
		void Hide();

		void SetAimPoint(const RefWrapper<SceneNode>& aimPoint, float aimRadius);
		void SetAimOffset(sf::Vector2f offset);
		[[nodiscard]] sf::Vector2f GetAimOffset() const;

	private:
		void UpdateAimPointPosition();

		RefWrapper<SceneNode> _aimPoint;
		float _aimRadius = 1.f;
		sf::Vector2f _aimOffset{};
		bool _isVisible = false;
	};

} // namespace Billiard
