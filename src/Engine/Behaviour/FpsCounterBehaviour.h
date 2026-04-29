#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <memory>

class SceneNode;

/// Плавный FPS в строке и отрисовка через TextVisual.
class FpsCounterBehaviour : public Behaviour
{
	META_CLASS()
public:
	explicit FpsCounterBehaviour(std::shared_ptr<sf::Text> text);

	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;
	void OnDeinit() override;

private:
	/// @property(readonly=true, name="FPS", tooltip="Smoothed frames per second (read-only).")
	float _fps = 0.f;
	/// @property(name="Filter", minValue=0.0, maxValue=1.0, step=0.001, dragSpeed=0.001, tooltip="Exponential smoothing
	/// factor for FPS display.")
	float _smoothFactor = 0.98f;
	/// @property(name="Demo offset (px)")
	sf::Vector2f _demoOffset{0.f, 0.f};
	/// @property(name="Demo Vec3")
	sf::Vector3f _demoVec3{0.f, 0.f, 0.f};
	/// @property(name="Text color")
	sf::Color _textColor{255, 255, 255, 255};
	std::shared_ptr<sf::Text> _text;
	bool _ownsDisplayVisual = false;
};

/// Обычная SceneNode с sf::Text и FpsCounterBehaviour.
std::shared_ptr<SceneNode> CreateFpsCounterNode();
