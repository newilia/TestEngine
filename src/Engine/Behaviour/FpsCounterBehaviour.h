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
	void OnDeinit() override;

private:
	/// @property
	sf::Color _textColor{255, 255, 255, 255};

	std::shared_ptr<sf::Text> _text;
	bool _ownsDisplayVisual = false;
};

std::shared_ptr<SceneNode> CreateFpsCounterNode();
