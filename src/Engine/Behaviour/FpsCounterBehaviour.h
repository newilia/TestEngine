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

class FpsCounterBehaviour : public Behaviour
{
	META_CLASS()
public:
	explicit FpsCounterBehaviour(std::shared_ptr<sf::Text> text);
	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;
	void OnPresent(const sf::Time& realFrameDt) override;
	void OnDeinit() override;

private:
	/// @property(readonly=true, name="FPS", tooltip="Smoothed frames per second (read-only).")
	float _fps = 0.f;
	/// @property(name="Filter", minValue=0.0, maxValue=1.0, step=0.001, dragSpeed=0.001)
	float _filteringFactor = 0.98f;
	/// @property(name="Text color")
	sf::Color _textColor{255, 255, 255, 255};

	float _tickHz = 0.f;
	std::shared_ptr<sf::Text> _text;
	bool _ownsDisplayVisual = false;
};

std::shared_ptr<SceneNode> CreateFpsCounterNode();
