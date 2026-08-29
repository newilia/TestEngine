#include "PoolBilliardUtils.h"

#include <fmt/format.h>

namespace Billiard::Utils {

	BallType GetBallType(int ballNumber) {
		if (ballNumber == 0) {
			return BallType::Cue;
		}
		if (ballNumber >= 1 && ballNumber <= 7) {
			return BallType::Solid;
		}
		if (ballNumber == 8) {
			return BallType::Eight;
		}
		if (ballNumber >= 9 && ballNumber <= 15) {
			return BallType::Striped;
		}
		return BallType::Undefined;
	}

	std::string FormatBallTexturePath(const std::string& pathMask, int ballIndex) {
		const std::string idPart = fmt::format("{}", ballIndex);
		const std::size_t placeholder = pathMask.find("{}");
		if (placeholder == std::string::npos) {
			return pathMask;
		}
		std::string result = pathMask;
		result.replace(placeholder, 2, idPart);
		return result;
	}

} // namespace Billiard::Utils
