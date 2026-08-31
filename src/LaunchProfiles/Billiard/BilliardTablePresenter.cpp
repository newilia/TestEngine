#include "BilliardTablePresenter.h"

#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "RollingBallBehaviour.h"

#include <algorithm>
#include <limits>
#include <optional>

namespace {

	constexpr float kBallOverlapEpsilon = 0.01f;
	constexpr float kPocketKeepOutMargin = 1.f;
	constexpr int kNearestFreePositionMaxIterations = 32;

	struct WorldCircle
	{
		sf::Vector2f center;
		float radius = 0.f;
	};

	std::optional<WorldCircle> TryGetCircleWorldGeometry(const CircleShapeVisual& circleVisual) {
		const auto node = circleVisual.GetNode();
		if (!node) {
			return std::nullopt;
		}
		const float localRadius = circleVisual.GetRadius();
		if (localRadius <= 0.f) {
			return std::nullopt;
		}

		sf::Transform combined = node->GetWorldTransform();
		if (const auto* visualTransform = circleVisual.GetTransform()) {
			combined *= *visualTransform;
		}

		const sf::Vector2f localCenter{localRadius, localRadius};
		const sf::Vector2f center = combined.transformPoint(localCenter);
		const float radius =
		    Utils::Length(combined.transformPoint(localCenter + sf::Vector2f{localRadius, 0.f}) - center);
		if (radius <= 0.f) {
			return std::nullopt;
		}
		return WorldCircle{center, radius};
	}

	[[nodiscard]] sf::Vector2f ClampBallCenterToRect(sf::Vector2f position, const sf::FloatRect& rect, float radius) {
		if (rect.size.x <= 0.f || rect.size.y <= 0.f) {
			return position;
		}
		const float minX = rect.position.x + radius;
		const float maxX = rect.position.x + rect.size.x - radius;
		const float minY = rect.position.y + radius;
		const float maxY = rect.position.y + rect.size.y - radius;
		position.x = std::clamp(position.x, minX, maxX);
		position.y = std::clamp(position.y, minY, maxY);
		return position;
	}

	[[nodiscard]] bool IsBallCenterInsideRect(const sf::Vector2f& position, const sf::FloatRect& rect, float radius) {
		if (rect.size.x <= 0.f || rect.size.y <= 0.f) {
			return true;
		}
		const float minX = rect.position.x + radius;
		const float maxX = rect.position.x + rect.size.x - radius;
		const float minY = rect.position.y + radius;
		const float maxY = rect.position.y + rect.size.y - radius;
		return position.x >= minX - kBallOverlapEpsilon && position.x <= maxX + kBallOverlapEpsilon &&
		       position.y >= minY - kBallOverlapEpsilon && position.y <= maxY + kBallOverlapEpsilon;
	}

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
			rect.size.x *= 0.25f;
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

	std::optional<sf::Vector2f> BilliardTablePresenter::GetNearestFreeBallPosition(
	    sf::Vector2f requestedPosition, int excludeBallNumber) const {
		const float ballRadius = GetBallRadius();
		if (ballRadius <= 0.f) {
			return requestedPosition;
		}

		struct KeepOutCircle
		{
			sf::Vector2f center;
			float minDistance;
		};

		std::vector<KeepOutCircle> keepOutCircles;
		keepOutCircles.reserve(_ballsBehaviours.size() + _pockets.size());

		sf::Transform tableLocalToWorld;
		sf::Transform worldToTableLocal;
		if (auto tableRectVisual = _tableRect.Get()) {
			if (auto tableNode = tableRectVisual->GetNode()) {
				tableLocalToWorld = tableNode->GetWorldTransform();
				worldToTableLocal = tableLocalToWorld.getInverse();
			}
		}

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
			keepOutCircles.push_back({Utils::GetWorldPos(node), ballRadius + ball->GetRadius()});
		}

		for (const auto& pocketRef : _pockets) {
			auto pocket = pocketRef.Get();
			if (!pocket) {
				continue;
			}
			auto pocketShape = pocket->GetPocketShape();
			if (!pocketShape) {
				continue;
			}
			if (const auto pocketCircle = TryGetCircleWorldGeometry(*pocketShape)) {
				keepOutCircles.push_back({pocketCircle->center, pocketCircle->radius + kPocketKeepOutMargin});
			}
		}

		sf::FloatRect tableRect = tableLocalToWorld.transformRect(GetBallInHandRect());
		requestedPosition = tableLocalToWorld.transformPoint(requestedPosition);
		requestedPosition = ClampBallCenterToRect(requestedPosition, tableRect, ballRadius);

		auto isFree = [&](const sf::Vector2f& position) {
			if (!IsBallCenterInsideRect(position, tableRect, ballRadius)) {
				return false;
			}
			for (const auto& other : keepOutCircles) {
				if (Utils::Length(position - other.center) < other.minDistance - kBallOverlapEpsilon) {
					return false;
				}
			}
			return true;
		};

		if (isFree(requestedPosition)) {
			return worldToTableLocal.transformPoint(requestedPosition);
		}

		sf::Vector2f result = requestedPosition;
		for (int iteration = 0; iteration < kNearestFreePositionMaxIterations; ++iteration) {
			bool moved = false;
			for (const auto& other : keepOutCircles) {
				const sf::Vector2f delta = result - other.center;
				const float distance = Utils::Length(delta);
				const float minDistance = other.minDistance;
				if (distance < minDistance - kBallOverlapEpsilon) {
					if (distance < std::numeric_limits<float>::epsilon()) {
						result = other.center + sf::Vector2f{minDistance, 0.f};
					}
					else {
						result = other.center + delta / distance * minDistance;
					}
					moved = true;
				}
			}
			const sf::Vector2f clamped = ClampBallCenterToRect(result, tableRect, ballRadius);
			if (clamped != result) {
				result = clamped;
				moved = true;
			}
			if (!moved) {
				break;
			}
		}

		result = ClampBallCenterToRect(result, tableRect, ballRadius);
		if (!isFree(result)) {
			return std::nullopt;
		}
		return worldToTableLocal.transformPoint(result);
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
			if (const auto freePosition = GetNearestFreeBallPosition(restorePosition, ballNumber)) {
				restorePosition = *freePosition;
			}
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
