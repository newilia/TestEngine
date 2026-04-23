#pragma once
#include "SceneNode.h"

#include <optional>

class FpsNode : public SceneNode
{
public:
	FpsNode();
	~FpsNode() override = default;
	void update(const sf::Time& dt) override;
	void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	float _fps = 0.f;
	std::optional<sf::Text> _text;
};
