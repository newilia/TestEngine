#pragma once

#include "BilliardTableSnapshot.h"
#include "BilliardTurnIntent.h"

namespace billiard {
	class TableSnapshotMsg;
	class TurnResultMsg;
	class TurnStartedMsg;
	class SubmitTurnIntentMsg;
} // namespace billiard

namespace Billiard {

	void FillTableSnapshotMsg(const TableSnapshot& snapshot, billiard::TableSnapshotMsg& message);
	[[nodiscard]] TableSnapshot ParseTableSnapshotMsg(const billiard::TableSnapshotMsg& message);

	void FillTurnStartedMsg(
	    std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot, billiard::TurnStartedMsg& message);
	void FillTurnResultMsg(
	    std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot, billiard::TurnResultMsg& message);
	void FillSubmitTurnIntentMsg(const TurnIntent& intent, billiard::SubmitTurnIntentMsg& message);
	[[nodiscard]] TurnIntent ParseSubmitTurnIntentMsg(const billiard::SubmitTurnIntentMsg& message);

} // namespace Billiard
