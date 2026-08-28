#include "AimGuideLineBehaviour.h"

#include "AimGuideLineBehaviour.generated.hpp"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNodeUtils.h"

#include <cmath>
#include <limits>
#include <optional>

namespace Billiard {

	namespace {
		constexpr float kDirectionEpsilon = 1e-5f;

		struct GhostBallHit
		{
			sf::Vector2f _ghostCenter{};
			sf::Vector2f _targetCenter{};
		};

		bool IsActiveBall(const std::shared_ptr<BilliardBallBehaviour>& ball) {
			if (!ball) {
				return false;
			}
			const auto node = ball->GetNode();
			return node && node->IsEnabled();
		}

		sf::Vector2f DirectionFromAngle(sf::Angle angle) {
			const float radians = angle.asRadians();
			return {std::cos(radians), std::sin(radians)};
		}

		std::optional<float> SolveGhostBallDistance(const sf::Vector2f& cueCenter, const sf::Vector2f& direction,
		    const sf::Vector2f& ballCenter, float twoRadii) {
			const sf::Vector2f offset = ballCenter - cueCenter;
			const float dotOffset = Utils::Dot(direction, offset);
			const float c = Utils::Sq(offset.x) + Utils::Sq(offset.y) - Utils::Sq(twoRadii);

			const auto roots = Utils::SolveQuadraticEquation(1.f, -2.f * dotOffset, c);
			if (!roots) {
				return std::nullopt;
			}

			float bestDistance = std::numeric_limits<float>::max();
			const auto consider = [&](float distance) {
				if (distance > kDirectionEpsilon && distance < bestDistance) {
					bestDistance = distance;
				}
			};

			consider(roots->first);
			if (roots->second) {
				consider(*roots->second);
			}

			if (bestDistance == std::numeric_limits<float>::max()) {
				return std::nullopt;
			}
			return bestDistance;
		}

		std::optional<GhostBallHit> FindFirstBallHit(const sf::Vector2f& cueCenter, const sf::Vector2f& direction,
		    float ballRadius, int cueBallIndex, const std::vector<std::weak_ptr<BilliardBallBehaviour>>& balls) {
			const float twoRadii = 2.f * ballRadius;
			std::optional<GhostBallHit> bestHit;
			float bestDistance = std::numeric_limits<float>::max();

			for (std::size_t ballIndex = 0; ballIndex < balls.size(); ++ballIndex) {
				if (static_cast<int>(ballIndex) == cueBallIndex) {
					continue;
				}

				const auto ball = balls[ballIndex].lock();
				if (!IsActiveBall(ball)) {
					continue;
				}

				const auto ballNode = ball->GetNode();
				const sf::Vector2f ballCenter = Utils::GetWorldPos(ballNode);
				const auto ghostDistance = SolveGhostBallDistance(cueCenter, direction, ballCenter, twoRadii);
				if (!ghostDistance || *ghostDistance >= bestDistance) {
					continue;
				}

				const sf::Vector2f ghostCenter = cueCenter + direction * *ghostDistance;
				const sf::Vector2f toTarget = ballCenter - ghostCenter;
				if (Utils::Dot(toTarget, direction) <= kDirectionEpsilon) {
					continue;
				}

				bestDistance = *ghostDistance;
				bestHit = GhostBallHit{ghostCenter, ballCenter};
			}

			return bestHit;
		}
	} // namespace

	void AimGuideLineBehaviour::SetBalls(std::vector<std::weak_ptr<BilliardBallBehaviour>> balls) {
		_balls = std::move(balls);
	}

	void AimGuideLineBehaviour::SetCueBallIndex(int cueBallIndex) {
		_cueBallIndex = cueBallIndex;
	}

	void AimGuideLineBehaviour::SetDirectionAngle(sf::Angle directionAngle) {
		_directionAngle = directionAngle;
	}

	void AimGuideLineBehaviour::Recalculate() {
		if (!_isGuideVisible) {
			return;
		}

		if (_cueBallIndex < 0 || static_cast<std::size_t>(_cueBallIndex) >= _balls.size()) {
			HideGuideVisuals();
			return;
		}

		const auto cueBall = _balls[_cueBallIndex].lock();
		if (!IsActiveBall(cueBall)) {
			HideGuideVisuals();
			return;
		}

		const float ballRadius = cueBall->GetRadius();
		if (ballRadius <= kDirectionEpsilon) {
			HideGuideVisuals();
			return;
		}

		const auto cueNode = cueBall->GetNode();
		const sf::Vector2f cueCenter = Utils::GetWorldPos(cueNode);
		const sf::Vector2f direction = DirectionFromAngle(_directionAngle);
		if (Utils::Length(direction) <= kDirectionEpsilon) {
			HideGuideVisuals();
			return;
		}

		const auto hit = FindFirstBallHit(cueCenter, direction, ballRadius, _cueBallIndex, _balls);
		if (!hit) {
			HideGuideVisuals();
			return;
		}

		const sf::Vector2f collisionNormal = Utils::Normalize(hit->_targetCenter - hit->_ghostCenter);
		const float cueTangentLength =
		    Utils::Length(direction - collisionNormal * Utils::Dot(direction, collisionNormal));
		const bool showCueDeflectRay = cueTangentLength > kDirectionEpsilon;

		if (auto imaginaryBall = _imaginaryBall.Get()) {
			if (auto imaginaryBallNode = imaginaryBall->GetNode()) {
				Utils::SetLocalPosToWorld(imaginaryBallNode, hit->_ghostCenter);
				imaginaryBallNode->SetVisible(true);
			}
			imaginaryBall->SetRadius(ballRadius);
		}

		const float ray1Length = Utils::Length(hit->_ghostCenter - cueCenter);
		if (auto ray1 = _ray1.Get()) {
			if (ray1Length > kDirectionEpsilon) {
				PlaceRay(ray1, cueCenter, direction, ray1Length);
			}
			else if (auto rayNode = ray1->GetNode()) {
				rayNode->SetVisible(false);
			}
		}

		if (auto ray2 = _ray2.Get()) {
			if (showCueDeflectRay) {
				const sf::Vector2f cueTangent =
				    Utils::Normalize(direction - collisionNormal * Utils::Dot(direction, collisionNormal));
				PlaceRay(ray2, hit->_ghostCenter, cueTangent, static_cast<float>(_maxRayLength));
			}
			else if (auto rayNode = ray2->GetNode()) {
				rayNode->SetVisible(false);
			}
		}

		if (auto ray3 = _ray3.Get()) {
			PlaceRay(ray3, hit->_targetCenter, collisionNormal, static_cast<float>(_maxRayLength));
		}
	}

	void AimGuideLineBehaviour::Show() {
		_isGuideVisible = true;
		Recalculate();
	}

	void AimGuideLineBehaviour::Hide() {
		_isGuideVisible = false;
		HideGuideVisuals();
	}

	void AimGuideLineBehaviour::HideGuideVisuals() {
		if (auto imaginaryBall = _imaginaryBall.Get()) {
			if (auto imaginaryBallNode = imaginaryBall->GetNode()) {
				imaginaryBallNode->SetVisible(false);
			}
		}
		if (auto ray1 = _ray1.Get()) {
			if (auto rayNode = ray1->GetNode()) {
				rayNode->SetVisible(false);
			}
		}
		if (auto ray2 = _ray2.Get()) {
			if (auto rayNode = ray2->GetNode()) {
				rayNode->SetVisible(false);
			}
		}
		if (auto ray3 = _ray3.Get()) {
			if (auto rayNode = ray3->GetNode()) {
				rayNode->SetVisible(false);
			}
		}
	}

	void AimGuideLineBehaviour::PlaceRay(const std::shared_ptr<RectangleShapeVisual>& visual,
	    const sf::Vector2f& worldStart, const sf::Vector2f& worldDir, float length) const {
		if (!visual || length <= kDirectionEpsilon) {
			return;
		}

		const float dirLength = Utils::Length(worldDir);
		if (dirLength <= kDirectionEpsilon) {
			return;
		}

		const sf::Vector2f unitDir = worldDir / dirLength;
		const auto rayNode = visual->GetNode();
		if (!rayNode) {
			return;
		}

		visual->SetSize({length, visual->GetSize().y});
		rayNode->SetLocalRotation(sf::radians(std::atan2(unitDir.y, unitDir.x)));
		Utils::SetLocalPosToWorld(rayNode, worldStart);
		rayNode->SetVisible(true);
	}

} // namespace Billiard
