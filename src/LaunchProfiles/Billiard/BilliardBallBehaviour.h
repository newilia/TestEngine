#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

namespace Billiard {

	class BilliardBallBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void SetBallNumber(int ballNumber);
		[[nodiscard]] int GetBallNumber() const;
		[[nodiscard]] bool IsCue() const;
		[[nodiscard]] bool IsEight() const;
		[[nodiscard]] bool IsStriped() const;

	private:
		/// @property(minValue=0, maxValue=15)
		int _ballNumber = 0;
	};

} // namespace Billiard
