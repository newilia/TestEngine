#pragma once
#include "SceneNode.h"

class FpsNode : public SceneNode {
public:
	FpsNode();
	~FpsNode() override = default;
	void update(const sf::Time& dt) override;
	void drawSelf(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	float mFps = 0.f;
	sf::Text mText;
};