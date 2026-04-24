#pragma once

#include "Engine/Core/Behaviour.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>

#include <memory>

class SceneNode;

/// Обычная SceneNode с sf::Text и FpsCounterBehaviour.
std::shared_ptr<SceneNode> CreateFpsCounterNode();

/// Плавный FPS в строке и отрисовка через FpsNodeVisual.
class FpsCounterBehaviour : public Behaviour
{
public:
	explicit FpsCounterBehaviour(std::shared_ptr<sf::Text> text);

	void OnAttached() override;

	void OnUpdate(const sf::Time& dt) override;

private:
	float _fps = 0.f;
	std::shared_ptr<sf::Text> _text;
};
