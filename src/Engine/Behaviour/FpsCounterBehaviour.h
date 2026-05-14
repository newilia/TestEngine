#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <memory>

class SceneNode;
class TextVisual;

class FpsCounterBehaviour : public Behaviour
{
	META_CLASS()

public:
	void OnUpdate(const sf::Time& dt) override;

	void SetTextVisual(std::shared_ptr<TextVisual> textVisual);

private:
	std::weak_ptr<TextVisual> _textVisual;
};

std::shared_ptr<SceneNode> CreateFpsCounterNode();
