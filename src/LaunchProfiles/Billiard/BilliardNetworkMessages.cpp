#include "BilliardNetworkMessages.h"

#include "BilliardSession.pb.h"

namespace Billiard {

	namespace {
		int BallTypeToProto(BallType type) {
			return static_cast<int>(type);
		}

		BallType BallTypeFromProto(int value) {
			return static_cast<BallType>(value);
		}

		int FoulKindToProto(FoulKind foulKind) {
			return static_cast<int>(foulKind);
		}

		FoulKind FoulKindFromProto(int value) {
			if (value < static_cast<int>(FoulKind::None) || value > static_cast<int>(FoulKind::TurnTimeOver)) {
				return FoulKind::None;
			}
			return static_cast<FoulKind>(value);
		}
	} // namespace

	void FillTableSnapshotMsg(const TableSnapshot& snapshot, billiard::TableSnapshotMsg& message) {
		message.clear_balls();
		for (const auto& ball : snapshot.balls) {
			auto* ballMsg = message.add_balls();
			ballMsg->set_ball_number(ball.ballNumber);
			ballMsg->set_pos_x(ball.position.x);
			ballMsg->set_pos_y(ball.position.y);
			ballMsg->set_vel_x(ball.velocity.x);
			ballMsg->set_vel_y(ball.velocity.y);
			ballMsg->set_angular_speed(ball.angularSpeed);
			ballMsg->set_is_on_table(ball.isOnTable);
			ballMsg->set_spin_omega_x(ball.spinOmega.x);
			ballMsg->set_spin_omega_y(ball.spinOmega.y);
			ballMsg->set_spin_omega_z(ball.spinOmega.z);
		}
	}

	TableSnapshot ParseTableSnapshotMsg(const billiard::TableSnapshotMsg& message) {
		TableSnapshot snapshot;
		snapshot.balls.reserve(static_cast<std::size_t>(message.balls_size()));
		for (const auto& ballMsg : message.balls()) {
			BallSnapshot ball;
			ball.ballNumber = ballMsg.ball_number();
			ball.position = {ballMsg.pos_x(), ballMsg.pos_y()};
			ball.velocity = {ballMsg.vel_x(), ballMsg.vel_y()};
			ball.angularSpeed = ballMsg.angular_speed();
			ball.isOnTable = ballMsg.is_on_table();
			ball.spinOmega = {ballMsg.spin_omega_x(), ballMsg.spin_omega_y(), ballMsg.spin_omega_z()};
			snapshot.balls.push_back(ball);
		}
		return snapshot;
	}

	void FillRulesSnapshotMsg(const RulesSnapshot& snapshot, billiard::RulesSnapshotMsg& message) {
		message.set_phase(static_cast<int32_t>(snapshot.phase));
		message.set_active_player(snapshot.activePlayerIndex);
		message.set_is_ball_in_hand(snapshot.isBallInHand);
		message.set_is_break_shot(snapshot.isBreakShot);
		message.clear_player_ball_types();
		for (const auto ballType : snapshot.playerBallTypes) {
			message.add_player_ball_types(BallTypeToProto(ballType));
		}
		message.clear_pocketed_solids();
		for (const auto ballNumber : snapshot.pocketedSolids) {
			message.add_pocketed_solids(ballNumber);
		}
		message.clear_pocketed_stripes();
		for (const auto ballNumber : snapshot.pocketedStripes) {
			message.add_pocketed_stripes(ballNumber);
		}
		message.set_is_game_over(snapshot.isGameOver);
		message.set_winner_index(snapshot.winnerIndex);
		message.set_foul_kind(FoulKindToProto(snapshot.foulKind));
	}

	RulesSnapshot ParseRulesSnapshotMsg(const billiard::RulesSnapshotMsg& message) {
		RulesSnapshot snapshot;
		snapshot.phase = static_cast<GamePhase>(message.phase());
		snapshot.activePlayerIndex = message.active_player();
		snapshot.isBallInHand = message.is_ball_in_hand();
		snapshot.isBreakShot = message.is_break_shot();
		for (int index = 0; index < message.player_ball_types_size() && index < 2; ++index) {
			snapshot.playerBallTypes[static_cast<std::size_t>(index)] =
			    BallTypeFromProto(message.player_ball_types(index));
		}
		for (const auto ballNumber : message.pocketed_solids()) {
			snapshot.pocketedSolids.insert(ballNumber);
		}
		for (const auto ballNumber : message.pocketed_stripes()) {
			snapshot.pocketedStripes.insert(ballNumber);
		}
		snapshot.isGameOver = message.is_game_over();
		snapshot.winnerIndex = message.winner_index();
		snapshot.foulKind = FoulKindFromProto(message.foul_kind());
		return snapshot;
	}

	void FillTurnStartedMsg(
	    std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot, billiard::TurnStartedMsg& message) {
		message.set_turn_id(turnId);
		message.set_active_player(activePlayer);
		FillTableSnapshotMsg(snapshot, *message.mutable_table());
	}

	void FillTurnResultMsg(std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot,
	    const RulesSnapshot& rules, billiard::TurnResultMsg& message) {
		message.set_turn_id(turnId);
		message.set_next_active_player(nextActivePlayer);
		FillTableSnapshotMsg(snapshot, *message.mutable_table());
		FillRulesSnapshotMsg(rules, *message.mutable_rules());
	}

	void FillCueAimUpdateMsg(
	    std::uint32_t turnId, int playerIndex, const TurnIntent& intent, billiard::CueAimUpdateMsg& message) {
		message.set_turn_id(turnId);
		message.set_player_index(playerIndex);
		message.set_direction_angle_rad(intent.directionAngle.asRadians());
		message.set_pull_distance(intent.pullDistance);
		message.set_lateral_spin(intent.lateralSpin);
		message.set_vertical_spin(intent.verticalSpin);
	}

	void FillTableStateUpdateMsg(
	    std::uint32_t turnId, int playerIndex, const TableSnapshot& snapshot, billiard::TableStateUpdateMsg& message) {
		message.set_turn_id(turnId);
		message.set_player_index(playerIndex);
		FillTableSnapshotMsg(snapshot, *message.mutable_table());
	}

	void FillSubmitTurnIntentMsg(const TurnIntent& intent, billiard::SubmitTurnIntentMsg& message) {
		message.set_turn_id(intent.turnId);
		message.set_player_index(intent.playerIndex);
		message.set_direction_angle_rad(intent.directionAngle.asRadians());
		message.set_pull_distance(intent.pullDistance);
		message.set_lateral_spin(intent.lateralSpin);
		message.set_vertical_spin(intent.verticalSpin);
	}

	TurnIntent ParseSubmitTurnIntentMsg(const billiard::SubmitTurnIntentMsg& message) {
		TurnIntent intent;
		intent.phase = TurnIntentPhase::Shoot;
		intent.turnId = message.turn_id();
		intent.playerIndex = message.player_index();
		intent.directionAngle = sf::radians(message.direction_angle_rad());
		intent.pullDistance = message.pull_distance();
		intent.lateralSpin = message.lateral_spin();
		intent.verticalSpin = message.vertical_spin();
		return intent;
	}

	TurnIntent ParseCueAimUpdateMsg(const billiard::CueAimUpdateMsg& message) {
		TurnIntent intent;
		intent.phase = TurnIntentPhase::Shoot;
		intent.turnId = message.turn_id();
		intent.playerIndex = message.player_index();
		intent.directionAngle = sf::radians(message.direction_angle_rad());
		intent.pullDistance = message.pull_distance();
		intent.lateralSpin = message.lateral_spin();
		intent.verticalSpin = message.vertical_spin();
		return intent;
	}

} // namespace Billiard
