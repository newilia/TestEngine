#pragma once
#include "SceneNode.h"

#include <optional>

class FpsNode : public SceneNode
{
public:
	FpsNode();
	~FpsNode() override = default;
	void Update(const sf::Time& dt) override;
	void DrawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	float _fps = 0.f;
	std::optional<sf::Text> _text;
};
