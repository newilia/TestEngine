#include "BilliardNetworkMessages.h"

#include "BilliardSession.pb.h"

namespace Billiard {

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

	void FillTurnStartedMsg(
	    std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot, billiard::TurnStartedMsg& message) {
		message.set_turn_id(turnId);
		message.set_active_player(activePlayer);
		FillTableSnapshotMsg(snapshot, *message.mutable_table());
	}

	void FillTurnResultMsg(
	    std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot, billiard::TurnResultMsg& message) {
		message.set_turn_id(turnId);
		message.set_next_active_player(nextActivePlayer);
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

} // namespace Billiard
