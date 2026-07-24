#include "BilliardBallBehaviour.h"

#include "BilliardBallBehaviour.generated.hpp"

namespace Billiard {

	void BilliardBallBehaviour::SetBallNumber(int ballNumber) {
		_ballNumber = ballNumber;
	}

	int BilliardBallBehaviour::GetBallNumber() const {
		return _ballNumber;
	}

	bool BilliardBallBehaviour::IsCue() const {
		return _ballNumber == 0;
	}

	bool BilliardBallBehaviour::IsEight() const {
		return _ballNumber == 8;
	}

	bool BilliardBallBehaviour::IsStriped() const {
		return _ballNumber >= 9 && _ballNumber <= 15;
	}

} // namespace Billiard
