#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/SubscriptionsHolderBase.h"

struct IntersectionDetails;

namespace Billiard {

	class BilliardBallSoundEventsBehaviour : public Behaviour, public SubscriptionsHolderBase
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnDeinit() override;

	private:
		void OnPhysicsContact(const IntersectionDetails& intersection, bool isOverlap);
	};

} // namespace Billiard
