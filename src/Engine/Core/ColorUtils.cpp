#include "Engine/Core/ColorUtils.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace {

	constexpr float kFillTargetSv = 0.75f;
	constexpr float kFillSvSpread = 0.1f;
	constexpr float kOutlineValue = 0.5f;

	float NextUnit01(std::mt19937& rng) {
		std::uniform_real_distribution<float> dist(0.f, 1.f);
		return dist(rng);
	}

	float NextNearTarget(std::mt19937& rng, float target, float spread) {
		const float minV = std::max(0.f, target - spread);
		const float maxV = std::min(1.f, target + spread);
		std::uniform_real_distribution<float> dist(minV, maxV);
		return dist(rng);
	}

	sf::Color HsvToRgb(float h, float s, float v) {
		h = h - std::floor(h);
		const float c = v * s;
		const float x = c * (1.f - std::fabs(std::fmod(h * 6.f, 2.f) - 1.f));
		const float m = v - c;
		float r = 0.f;
		float g = 0.f;
		float b = 0.f;
		const int sector = static_cast<int>(h * 6.f) % 6;
		switch (sector) {
		case 0:
			r = c;
			g = x;
			break;
		case 1:
			r = x;
			g = c;
			break;
		case 2:
			g = c;
			b = x;
			break;
		case 3:
			g = x;
			b = c;
			break;
		case 4:
			r = x;
			b = c;
			break;
		default:
			r = c;
			b = x;
			break;
		}
		return sf::Color(static_cast<std::uint8_t>((r + m) * 255.f), static_cast<std::uint8_t>((g + m) * 255.f),
		    static_cast<std::uint8_t>((b + m) * 255.f), 255);
	}

} // namespace

Utils::HsvShapeColors Utils::RandomHsvShapeColors() {
	thread_local std::mt19937 rng{std::random_device{}()};
	const float hue = NextUnit01(rng);
	const float saturation = NextNearTarget(rng, kFillTargetSv, kFillSvSpread);
	const float fillValue = NextNearTarget(rng, kFillTargetSv, kFillSvSpread);
	return {HsvToRgb(hue, saturation, fillValue), HsvToRgb(hue, saturation, kOutlineValue)};
}
