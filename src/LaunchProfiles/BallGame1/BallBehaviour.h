#pragma once
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

namespace BallGame1 {
	META_ENUM(BallType, Type1, Type2, Type3, Type4, Type5);

	class BallBehaviour : public Behaviour
	{
		META_CLASS()
	public:
		BallType GetType() const {
			return _type;
		}

	private:
		/// @property
		BallType _type;
	};
} // namespace BallGame1
