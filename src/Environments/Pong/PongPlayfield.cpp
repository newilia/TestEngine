#include "PongPlayfield.h"

#include "Engine/App/MainContext.h"

namespace {
	/// Fraction of the window size used as margin on each side (symmetric inset).
	const sf::Vector2f fieldPaddings(0.35f, 0.0f);
} // namespace

sf::FloatRect GetPongPlayfieldRect() {
	const auto screenSize = sf::Vector2f(Engine::MainContext::GetInstance().GetMainWindow()->getSize());
	const sf::Vector2f margin = screenSize.componentWiseMul(fieldPaddings);
	const sf::Vector2f innerSize = screenSize - margin * 2.f;
	return {margin, innerSize};
}

sf::Vector2f GetPongWindowSize() {
	return sf::Vector2f(Engine::MainContext::GetInstance().GetMainWindow()->getSize());
}
