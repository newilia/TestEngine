#pragma once

#include "BallType.h"

#include <string>

namespace Billiard {
	namespace Utils {
		[[nodiscard]] BallType GetBallType(int ballNumber);
		[[nodiscard]] std::string FormatBallTexturePath(const std::string& pathMask, int ballIndex);

	} // namespace Utils

} // namespace Billiard
