#include "BilliardTablePresenter.h"

#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "RollingBallBehaviour.h"

#include <limits>

namespace {

	constexpr float kBallOverlapEpsilon = 0.01f;
	constexpr int kNearestFreePositionMaxIterations = 16;

} // namespace

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

	sf::Vector2f BilliardTablePresenter::GetEightBallRestorePosition() const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->GetGlobalBounds().size.componentWiseMul({0.75f, 0.5f});
		}
		return sf::Vector2f();
	}

	sf::Vector2f BilliardTablePresenter::GetNearestFreeBallPosition(
	    sf::Vector2f requestedPosition, int excludeBallNumber) const {
		const float ballRadius = GetBallRadius();
		if (ballRadius <= 0.f) {
			return requestedPosition;
		}

		struct OccupiedBall
		{
			sf::Vector2f position;
			float radius;
		};

		std::vector<OccupiedBall> occupiedBalls;
		occupiedBalls.reserve(_ballsBehaviours.size());

		for (const auto& [ballNumber, ballRef] : _ballsBehaviours) {
			if (ballNumber == excludeBallNumber) {
				continue;
			}
			auto ball = ballRef.Get();
			if (!ball) {
				continue;
			}
			auto node = ball->GetNode();
			if (!node || !node->IsEnabled()) {
				continue;
			}
			occupiedBalls.push_back({node->GetLocalPosition(), ball->GetRadius()});
		}

		auto isFree = [&](const sf::Vector2f& position) {
			for (const auto& other : occupiedBalls) {
				if (Utils::Length(position - other.position) < ballRadius + other.radius - kBallOverlapEpsilon) {
					return false;
				}
			}
			return true;
		};

		if (isFree(requestedPosition)) {
			return requestedPosition;
		}

		sf::Vector2f result = requestedPosition;
		for (int iteration = 0; iteration < kNearestFreePositionMaxIterations; ++iteration) {
			bool moved = false;
			for (const auto& other : occupiedBalls) {
				const sf::Vector2f delta = result - other.position;
				const float distance = Utils::Length(delta);
				const float minDistance = ballRadius + other.radius;
				if (distance < minDistance - kBallOverlapEpsilon) {
					if (distance < std::numeric_limits<float>::epsilon()) {
						result = other.position + sf::Vector2f{minDistance, 0.f};
					}
					else {
						result = other.position + delta / distance * minDistance;
					}
					moved = true;
				}
			}
			if (!moved) {
				break;
			}
		}

		return result;
	}

	void BilliardTablePresenter::OnBallPocketed(int ballNumber, shared_ptr<BilliardPocketBehaviour> pocket) {
		if (auto ball = _ballsBehaviours[ballNumber].Get()) {
			if (auto physicsBody = ball->GetPhysicsBody(); physicsBody && pocket) {
				physicsBody->GetCollisionGroups() = {};
				physicsBody->GetCollisionGroups()[pocket->UseBallCollisionGroup()] = true;
			}
			ball->PlayFallAnimation();
		}
	}

	void BilliardTablePresenter::RestoreBall(int ballNumber) {
		if (auto ball = _ballsBehaviours[ballNumber].Get()) {
			auto restorePosition = ballNumber == 8 ? GetEightBallRestorePosition() : GetTableCenter();
			restorePosition = GetNearestFreeBallPosition(restorePosition);
			ball->GetNode()->SetLocalPosition(restorePosition);
			ball->RestoreCollisionGroups();
			ball->Appear();

			if (auto physicsBody = ball->GetPhysicsBody()) {
				physicsBody->SetVelocity(sf::Vector2f(0, 0));
				physicsBody->SetAngularSpeed(0.f);
			}

			if (auto rollingBall = ball->GetRollingBallBehaviour()) {
				rollingBall->ResetOmega();
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
