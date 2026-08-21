#include "BilliardPocketBehaviour.h"

#include "BilliardPocketBehaviour.generated.hpp"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <optional>

namespace {
	constexpr float kPocketPullSpeed = 5000.f;

	struct WorldCircle
	{
		sf::Vector2f center;
		float radius = 0.f;
	};

	// looks like overhead
	[[nodiscard]] std::optional<WorldCircle> GetCircleWorldGeometry(const CircleShapeVisual& circleVisual) {
		const auto node = circleVisual.GetNode();
		const auto* circle = dynamic_cast<const sf::CircleShape*>(circleVisual.GetBaseShape());
		if (!node || !circle) {
			return std::nullopt;
		}

		sf::Transform combined = node->GetWorldTransform();
		combined *= circle->getTransform();
		const sf::Vector2f localCenter = circle->getGeometricCenter();
		const sf::Vector2f center = combined.transformPoint(localCenter);
		const float radius =
		    Utils::Length(combined.transformPoint(localCenter + sf::Vector2f{circle->getRadius(), 0.f}) - center);
		if (radius <= 0.f) {
			return std::nullopt;
		}
		return WorldCircle{center, radius};
	}

	[[nodiscard]] bool IsPointInsideCircle(
	    const sf::Vector2f& point, const sf::Vector2f& circleCenter, float circleRadius) {
		return Utils::Length(point - circleCenter) <= circleRadius;
	}

	[[nodiscard]] bool IsCircleCompletelyInsideCircle(
	    const sf::Vector2f& innerCenter, float innerRadius, const sf::Vector2f& outerCenter, float outerRadius) {
		return Utils::Length(innerCenter - outerCenter) + innerRadius <= outerRadius;
	}

	[[nodiscard]] float GetBallWorldRadius(const std::shared_ptr<const SceneNode>& ballNode) {
		if (auto circleShape = ballNode->GetVisual<CircleShapeVisual>()) {
			return circleShape->GetRadius();
		}
		return 0.f;
	}

} // namespace

namespace Billiard {

	void BilliardPocketBehaviour::OnBallPocketed(BilliardBallBehaviour& ballBehaviour) {
		const int ballNumber = ballBehaviour.GetBallNumber();
		if (_fallenBalls.contains(ballNumber)) {
			return;
		}
		_fallenBalls.insert(ballNumber);
		_onBallPocketedSignal.Emit(ballNumber);
	}

	int BilliardPocketBehaviour::UseBallCollisionGroup() {
		auto group = _nextBallCollisionGroup++;
		if (_nextBallCollisionGroup == PhysicsBodyBehaviour::kGroupsCount - 1) {
			_nextBallCollisionGroup = 1;
		}
		return group;
	}

	void BilliardPocketBehaviour::OnUpdate(const sf::Time& dt) {
		const auto pocketShape = _pocketShape.Get();
		if (!pocketShape) {
			return;
		}

		const auto pocketNode = pocketShape->GetNode();
		if (!pocketNode) {
			return;
		}

		const auto pocketGeometry = GetCircleWorldGeometry(*pocketShape);
		if (!pocketGeometry) {
			return;
		}

		const auto pocketBounds = Utils::TryGetNodeVisualWorldBounds(pocketNode);
		if (!pocketBounds) {
			return;
		}

		for (auto& ballRef : _balls) {
			const auto ballBehaviour = ballRef.Get();
			if (!ballBehaviour) {
				continue;
			}
			const auto ballNode = ballBehaviour->GetNode();
			if (!ballNode) {
				continue;
			}
			const auto ballBounds = Utils::TryGetNodeVisualWorldBounds(ballNode);
			if (!ballBounds) {
				continue;
			}
			const int ballNumber = ballBehaviour->GetBallNumber();

			if (pocketBounds->findIntersection(*ballBounds)) {
				const sf::Vector2f ballCenter = Utils::GetWorldPos(ballNode);

				if (IsPointInsideCircle(ballCenter, pocketGeometry->center, pocketGeometry->radius)) {
					if (_fallenBalls.contains(ballNumber)) {
						continue;
					}

					const auto ballBody = ballNode->FindBehaviour<PhysicsBodyBehaviour>();
					if (!ballBody) {
						continue;
					}
					const float ballRadius = GetBallWorldRadius(ballNode);

					if (IsCircleCompletelyInsideCircle(
					        ballCenter, ballRadius, pocketGeometry->center, pocketGeometry->radius)) {
						OnBallPocketed(*ballBehaviour);
					}
					else {
						const sf::Vector2f toPocket = pocketGeometry->center - ballCenter;
						const float dist = Utils::Length(toPocket);
						if (dist > 1e-6f) {
							float sinArg = 3.14 * (pocketGeometry->radius - dist) / (ballRadius);
							float accelerationMag = sin(sinArg) * kPocketPullSpeed;
							ballBody->AddVelocity(accelerationMag * Utils::Normalize(toPocket) * dt.asSeconds());
						}
					}
				}
				else {
					if (_fallenBalls.contains(ballNumber)) {
						_fallenBalls.erase(ballNumber);
					}
				}
			}
			else {
				if (_fallenBalls.contains(ballNumber)) {
					_fallenBalls.erase(ballNumber);
					continue;
				}
			}
		}
	}

	void BilliardPocketBehaviour::Reset() {
		_balls.clear();
		_fallenBalls.clear();
		_nextBallCollisionGroup = 1;
	}

	void BilliardPocketBehaviour::RegisterBall(const BilliardBallBehaviour& ball) {
		RefWrapper<BilliardBallBehaviour> ballRef;
		ballRef.SetId(ball.GetEntityId());
		_balls.push_back(ballRef);
	}

	Signal<int>& BilliardPocketBehaviour::GetOnBallPocketedSignal() const {
		return _onBallPocketedSignal;
	}

} // namespace Billiard
