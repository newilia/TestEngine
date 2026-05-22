#include "PongPlayfield.h"

#include "Engine/Core/MainContext.h"

#include <optional>

namespace {
	/// Fraction of the window size used as margin on each side (symmetric inset).
	const sf::Vector2f fieldPaddings(0.35f, 0.0f);

	std::optional<sf::FloatRect> playfieldRectOverride;
} // namespace

void SetPongPlayfieldRectOverride(const sf::FloatRect& rect) {
	playfieldRectOverride = rect;
}

void ClearPongPlayfieldRectOverride() {
	playfieldRectOverride.reset();
}

sf::FloatRect GetPongPlayfieldRect() {
	if (playfieldRectOverride) {
		return *playfieldRectOverride;
	}
	const auto screenSize = sf::Vector2f(Engine::MainContext::GetInstance().GetMainWindow()->getSize());
	const sf::Vector2f margin = screenSize.componentWiseMul(fieldPaddings);
	const sf::Vector2f innerSize = screenSize - margin * 2.f;
	return {margin, innerSize};
}

sf::Vector2f GetPongWindowSize() {
	return sf::Vector2f(Engine::MainContext::GetInstance().GetMainWindow()->getSize());
}
