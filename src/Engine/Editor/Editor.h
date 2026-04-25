#pragma once

#include "Engine/Core/Singleton.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace Engine {
class Editor : public Singleton<Editor>
{
public:
	void Toggle();
	bool IsOpen() const;
	void Update(float dt);
	void Draw();
	void OnEvent(const sf::Event& event);
	void OnResize(const sf::Vector2u& size);
	void OnKeyPress(const sf::Keyboard::Key& key);
	void OnKeyRelease(const sf::Keyboard::Key& key);
	void OnMouseMove(const sf::Vector2i& position);

private:
	bool _isOpen = false;
};
} // namespace Engine
