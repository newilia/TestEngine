#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class Visual : public EntityOnNode
{
public:
	~Visual() override = default;

	virtual void Draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

	virtual bool HitTest(sf::Vector2f windowPosition) const = 0;

	virtual void OnTap(sf::Vector2f windowPosition) { (void)windowPosition; }

	void SetTapHandlingEnabled(bool enabled) { _tapHandlingEnabled = enabled; }

	bool IsTapHandlingEnabled() const { return _tapHandlingEnabled; }

	void SetTransparentToTap(bool transparent) { _transparentToTap = transparent; }

	bool IsTransparentToTap() const { return _transparentToTap; }

private:
	bool _tapHandlingEnabled = false;
	bool _transparentToTap = false;
};
