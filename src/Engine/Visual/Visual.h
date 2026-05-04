#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class Visual : public EntityOnNode
{
	META_CLASS()
public:
	~Visual() override = default;

	virtual void Draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	virtual bool HitTest(const sf::Vector2f& worldPoint) const = 0;
	virtual void OnTap(const sf::Vector2f& worldPoint); // TODO Remove, I guess this is not a Visual's responsibility

	void SetTapHandlingEnabled(bool enabled);
	bool IsTapHandlingEnabled() const;

private:
	/// @property
	bool _tapHandlingEnabled = false;
};
