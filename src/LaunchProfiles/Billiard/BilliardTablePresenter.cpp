#include "BilliardTablePresenter.h"

#include "Engine/Core/SceneNodeUtils.h"
#include "RollingBallBehaviour.h"

namespace Billiard {

	void BilliardTablePresenter::SetBalls(std::map<int, RefWrapper<BilliardBallBehaviour>> balls) {
		_ballsBehaviours = std::move(balls);
	}

	void BilliardTablePresenter::SetTableRect(RefWrapper<RectangleShapeVisual> tableRect) {
		_tableRect = std::move(tableRect);
	}

	void BilliardTablePresenter::SetPockets(std::vector<RefWrapper<BilliardPocketBehaviour>> pockets) {
		_pockets = std::move(pockets);
	}

	TableSnapshot BilliardTablePresenter::CaptureSnapshot() const {
		TableSnapshot snapshot;
		snapshot.balls.reserve(_ballsBehaviours.size());
		for (const auto& [ballNumber, ballRef] : _ballsBehaviours) {
			if (auto ball = ballRef.Get()) {
				BallSnapshot ballSnapshot;
				ballSnapshot.ballNumber = ballNumber;
				ballSnapshot.isOnTable = ball->GetNode() && ball->GetNode()->IsEnabled();
				if (auto node = ball->GetNode()) {
					ballSnapshot.position = node->GetLocalPosition();
				}
				if (auto physicsBody = ball->GetPhysicsBody()) {
					ballSnapshot.velocity = physicsBody->GetVelocity();
					ballSnapshot.angularSpeed = physicsBody->GetAngularSpeed();
				}
				if (auto rollingBall = ball->GetRollingBallBehaviour()) {
					ballSnapshot.spinOmega = rollingBall->GetSpinOmega();
				}
				snapshot.balls.push_back(ballSnapshot);
			}
		}
		return snapshot;
	}

	void BilliardTablePresenter::ApplySnapshot(const TableSnapshot& snapshot) {
		for (const auto& ballSnapshot : snapshot.balls) {
			auto it = _ballsBehaviours.find(ballSnapshot.ballNumber);
			if (it == _ballsBehaviours.end()) {
				continue;
			}
			auto ball = it->second.Get();
			if (!ball) {
				continue;
			}
			if (ballSnapshot.isOnTable) {
				ball->Appear();
				if (auto node = ball->GetNode()) {
					node->SetLocalPosition(ballSnapshot.position);
				}
				if (auto physicsBody = ball->GetPhysicsBody()) {
					physicsBody->SetVelocity(ballSnapshot.velocity);
					physicsBody->SetAngularSpeed(ballSnapshot.angularSpeed);
				}
				if (auto rollingBall = ball->GetRollingBallBehaviour()) {
					rollingBall->SetSpinOmega(ballSnapshot.spinOmega);
				}
			}
			else if (auto node = ball->GetNode()) {
				node->SetEnabled(false);
			}
		}
	}

	bool BilliardTablePresenter::AreBallsMoving() const {
		for (const auto& [_, ball] : _ballsBehaviours) {
			if (auto ballBehaviour = ball.Get()) {
				if (!ballBehaviour->GetNode()->IsEnabled()) {
					continue;
				}
				if (auto physicsBody = ballBehaviour->GetPhysicsBody()) {
					if (physicsBody->GetVelocity().length() > 1.f || std::abs(physicsBody->GetAngularSpeed()) > 0.1f) {
						return true;
					}
				}
			}
		}
		return false;
	}

	sf::FloatRect BilliardTablePresenter::GetBallInHandRect() const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->GetGlobalBounds();
		}
		return sf::FloatRect();
	}

	sf::FloatRect BilliardTablePresenter::GetKitchenRect() const {
		if (auto tableRect = _tableRect.Get()) {
			auto rect = tableRect->GetGlobalBounds();
			auto radius = GetBallRadius();
			rect.size.x *= 0.25f;
			rect.size.x += radius;
			return rect;
		}
		return sf::FloatRect();
	}

	float BilliardTablePresenter::GetBallRadius() const {
		if (_ballsBehaviours.empty()) {
			return 0.f;
		}
		auto ball = _ballsBehaviours.begin()->second.Get();
		if (!ball) {
			return 0.f;
		}
		return ball->GetRadius();
	}

	sf::Vector2f BilliardTablePresenter::GetTableCenter() const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->GetGlobalBounds().size * 0.5f;
		}
		return sf::Vector2f();
	}

	void BilliardTablePresenter::RestoreBall(int ballNumber) {
		if (auto ball = _ballsBehaviours[ballNumber].Get()) {
			ball->Appear();
			ball->GetNode()->SetLocalPosition(GetTableCenter()); // todo find proper position
			ball->RestoreCollisionGroups();

			if (auto physicsBody = ball->GetPhysicsBody()) {
				physicsBody->SetVelocity(sf::Vector2f(0, 0));
				physicsBody->SetAngularSpeed(0.f);
			}

			if (auto rollingBall = ball->GetRollingBallBehaviour()) {
				rollingBall->ResetOmega();
			}
		}
		for (auto& pocket : _pockets) {
			if (auto pocketBehaviour = pocket.Get()) {
				pocketBehaviour->UnpocketBall(ballNumber);
			}
		}
	}

	bool BilliardTablePresenter::IsBallOutsideExpandedTable(int ballNumber, float margin) const {
		auto it = _ballsBehaviours.find(ballNumber);
		if (it == _ballsBehaviours.end()) {
			return false;
		}
		auto ball = it->second.Get();
		if (!ball) {
			return false;
		}

		sf::FloatRect allowedBounds = GetBallInHandRect();
		allowedBounds.position -= {margin, margin};
		allowedBounds.size += {2.f * margin, 2.f * margin};

		const auto ballNode = ball->GetNode();
		if (!ballNode || !ballNode->IsEnabled()) {
			return false;
		}

		const sf::Vector2f ballCenter = ballNode->GetLocalTransform().GetPosition();
		if (!allowedBounds.contains(ballCenter)) {
			return true;
		}
		return false;
	}

} // namespace Billiard
