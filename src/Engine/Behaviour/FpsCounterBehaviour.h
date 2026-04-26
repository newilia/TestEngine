#pragma once

#include "Engine/Behaviour/Behaviour.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
	class PropertyBuilder;
}

class SceneNode;

/// Обычная SceneNode с sf::Text и FpsCounterBehaviour.
std::shared_ptr<SceneNode> CreateFpsCounterNode();

/// Плавный FPS в строке и отрисовка через TextVisual.
class FpsCounterBehaviour : public Behaviour
{
public:
	explicit FpsCounterBehaviour(std::shared_ptr<sf::Text> text);

	void OnAttached() override;

	void OnUpdate(const sf::Time& dt) override;

	void BuildPropertyTree(Engine::PropertyBuilder& builder) override;

private:
	float _fps = 0.f;
	float _smoothFactor = 0.98f;
	sf::Vector2f _demoOffset{0.f, 0.f};
	sf::Vector3f _demoVec3{0.f, 0.f, 0.f};
	sf::Color _textColor{255, 255, 255, 255};
	std::vector<float> _curveSamples{0.25f, 0.5f, 0.75f};
	std::map<std::string, float> _demoStats{{"hp", 100.f}, {"energy", 50.f}};
	std::shared_ptr<sf::Text> _text;
};
