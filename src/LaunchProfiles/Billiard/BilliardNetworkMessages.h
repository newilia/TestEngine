#pragma once

#include "BilliardRulesSnapshot.h"
#include "BilliardTableSnapshot.h"
#include "BilliardTurnIntent.h"

namespace billiard {
	class BallInHandDragEndedMsg;
	class BallInHandDragStartedMsg;
	class CueAimUpdateMsg;
	class CueReleasedMsg;
	class RulesSnapshotMsg;
	class TableSnapshotMsg;
	class TableStateUpdateMsg;
	class TurnResultMsg;
	class TurnStartedMsg;
	class SubmitTurnIntentMsg;
} // namespace billiard

namespace Billiard {

	void FillTableSnapshotMsg(const TableSnapshot& snapshot, billiard::TableSnapshotMsg& message);
	[[nodiscard]] TableSnapshot ParseTableSnapshotMsg(const billiard::TableSnapshotMsg& message);

	void FillRulesSnapshotMsg(const RulesSnapshot& snapshot, billiard::RulesSnapshotMsg& message);
	[[nodiscard]] RulesSnapshot ParseRulesSnapshotMsg(const billiard::RulesSnapshotMsg& message);

	void FillTurnStartedMsg(
	    std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot, billiard::TurnStartedMsg& message);
	void FillTurnResultMsg(std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot,
	    const RulesSnapshot& rules, billiard::TurnResultMsg& message);
	void FillCueAimUpdateMsg(
	    std::uint32_t turnId, int playerIndex, const TurnIntent& intent, billiard::CueAimUpdateMsg& message);
	void FillTableStateUpdateMsg(
	    std::uint32_t turnId, int playerIndex, const TableSnapshot& snapshot, billiard::TableStateUpdateMsg& message);
	void FillBallInHandDragStartedMsg(
	    std::uint32_t turnId, int playerIndex, billiard::BallInHandDragStartedMsg& message);
	void FillBallInHandDragEndedMsg(std::uint32_t turnId, int playerIndex, billiard::BallInHandDragEndedMsg& message);
	void FillCueReleasedMsg(
	    std::uint32_t turnId, int playerIndex, const TurnIntent& intent, billiard::CueReleasedMsg& message);
	void FillSubmitTurnIntentMsg(const TurnIntent& intent, billiard::SubmitTurnIntentMsg& message);
	[[nodiscard]] TurnIntent ParseSubmitTurnIntentMsg(const billiard::SubmitTurnIntentMsg& message);
	[[nodiscard]] TurnIntent ParseCueAimUpdateMsg(const billiard::CueAimUpdateMsg& message);
	[[nodiscard]] TurnIntent ParseCueReleasedMsg(const billiard::CueReleasedMsg& message);

} // namespace Billiard
