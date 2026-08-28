#include "AimGuideLineBehaviour.h"

#include "AimGuideLineBehaviour.generated.hpp"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNodeUtils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>

namespace Billiard {

	namespace {
		constexpr float kDirectionEpsilon = 1e-5f;
		constexpr float kRailContactTolerance = 1e-3f;

		struct GhostBallHit
		{
			sf::Vector2f _ghostCenter{};
			sf::Vector2f _targetCenter{};
			float _distance = 0.f;
		};

		struct RailHit
		{
			sf::Vector2f _ghostCenter{};
			float _distance = 0.f;
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
				bestHit = GhostBallHit{ghostCenter, ballCenter, *ghostDistance};
			}

			return bestHit;
		}

		std::array<sf::Vector2f, 4> GetRectangleWorldCorners(const RectangleShapeVisual& visual) {
			const sf::FloatRect bounds = visual.GetLocalBounds();
			const std::array<sf::Vector2f, 4> localCorners = {
			    bounds.position,
			    {bounds.position.x + bounds.size.x, bounds.position.y},
			    {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y},
			    {bounds.position.x, bounds.position.y + bounds.size.y},
			};

			const auto node = visual.GetNode();
			sf::Transform transform = node->GetWorldTransform();
			if (const auto* visualTransform = visual.GetTransform()) {
				transform *= *visualTransform;
			}

			std::array<sf::Vector2f, 4> worldCorners{};
			for (std::size_t cornerIndex = 0; cornerIndex < worldCorners.size(); ++cornerIndex) {
				worldCorners[cornerIndex] = transform.transformPoint(localCorners[cornerIndex]);
			}
			return worldCorners;
		}

		std::optional<float> FindRaySegmentCircleContactDistance(const sf::Vector2f& origin, const sf::Vector2f& dir,
		    const sf::Vector2f& segStart, const sf::Vector2f& segEnd, float radius) {
			const sf::Vector2f segment = segEnd - segStart;
			const float segmentLengthSq = Utils::Dot(segment, segment);
			if (segmentLengthSq <= kDirectionEpsilon) {
				return std::nullopt;
			}

			const float segmentLength = std::sqrt(segmentLengthSq);
			const sf::Vector2f segmentDir = segment / segmentLength;

			const sf::Vector2f offsetToStart = origin - segStart;
			const sf::Vector2f dirPerp = dir - segmentDir * Utils::Dot(dir, segmentDir);
			const sf::Vector2f offsetPerp = offsetToStart - segmentDir * Utils::Dot(offsetToStart, segmentDir);

			const float dirPerpLengthSq = Utils::Dot(dirPerp, dirPerp);
			if (dirPerpLengthSq <= kDirectionEpsilon) {
				return std::nullopt;
			}

			const auto roots = Utils::SolveQuadraticEquation(dirPerpLengthSq, 2.f * Utils::Dot(offsetPerp, dirPerp),
			    Utils::Dot(offsetPerp, offsetPerp) - Utils::Sq(radius));
			if (!roots) {
				return std::nullopt;
			}

			float bestDistance = std::numeric_limits<float>::max();
			const auto consider = [&](float distance) {
				if (distance <= kDirectionEpsilon || distance >= bestDistance) {
					return;
				}

				const sf::Vector2f contactCenter = origin + dir * distance;
				const float alongSegment =
				    std::clamp(Utils::Dot(contactCenter - segStart, segmentDir), 0.f, segmentLength);
				const sf::Vector2f closestPoint = segStart + segmentDir * alongSegment;
				const float centerDistance = Utils::Length(contactCenter - closestPoint);
				if (std::abs(centerDistance - radius) <= radius * kRailContactTolerance + kDirectionEpsilon) {
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

		std::optional<RailHit> FindFirstRailHit(const sf::Vector2f& cueCenter, const sf::Vector2f& direction,
		    float ballRadius, const std::vector<RefWrapper<RectangleShapeVisual>>& rails) {
			std::optional<RailHit> bestHit;
			float bestDistance = std::numeric_limits<float>::max();

			for (const auto& railRef : rails) {
				const auto rail = railRef.Get();
				if (!rail) {
					continue;
				}

				const auto corners = GetRectangleWorldCorners(*rail);
				for (std::size_t edgeIndex = 0; edgeIndex < corners.size(); ++edgeIndex) {
					const auto edgeDistance = FindRaySegmentCircleContactDistance(cueCenter, direction,
					    corners[edgeIndex], corners[(edgeIndex + 1) % corners.size()], ballRadius);
					if (!edgeDistance || *edgeDistance >= bestDistance) {
						continue;
					}

					bestDistance = *edgeDistance;
					bestHit = RailHit{cueCenter + direction * *edgeDistance, *edgeDistance};
				}
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

		const auto ballHit = FindFirstBallHit(cueCenter, direction, ballRadius, _cueBallIndex, _balls);
		const auto railHit = FindFirstRailHit(cueCenter, direction, ballRadius, _tableRails);

		const bool isBallHitFirst = ballHit && (!railHit || ballHit->_distance < railHit->_distance);
		const bool isRailHitFirst = railHit && (!ballHit || railHit->_distance < ballHit->_distance);

		if (!isBallHitFirst && !isRailHitFirst) {
			HideGuideVisuals();
			return;
		}

		if (isRailHitFirst) {
			if (auto imaginaryBall = _imaginaryBall.Get()) {
				if (auto imaginaryBallNode = imaginaryBall->GetNode()) {
					Utils::SetLocalPosToWorld(imaginaryBallNode, railHit->_ghostCenter);
					imaginaryBallNode->SetVisible(true);
				}
				imaginaryBall->SetRadius(ballRadius);
			}

			const float ray1Length = Utils::Length(railHit->_ghostCenter - cueCenter);
			if (auto ray1 = _ray1.Get()) {
				if (ray1Length > kDirectionEpsilon) {
					PlaceRay(ray1, cueCenter, direction, ray1Length);
				}
				else if (auto rayNode = ray1->GetNode()) {
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
			return;
		}

		const sf::Vector2f collisionNormal = Utils::Normalize(ballHit->_targetCenter - ballHit->_ghostCenter);
		const float cueTangentLength =
		    Utils::Length(direction - collisionNormal * Utils::Dot(direction, collisionNormal));
		const bool showCueDeflectRay = cueTangentLength > kDirectionEpsilon;

		if (auto imaginaryBall = _imaginaryBall.Get()) {
			if (auto imaginaryBallNode = imaginaryBall->GetNode()) {
				Utils::SetLocalPosToWorld(imaginaryBallNode, ballHit->_ghostCenter);
				imaginaryBallNode->SetVisible(true);
			}
			imaginaryBall->SetRadius(ballRadius);
		}

		const float ray1Length = Utils::Length(ballHit->_ghostCenter - cueCenter);
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
				PlaceRay(ray2, ballHit->_ghostCenter, cueTangent, static_cast<float>(_maxRayLength));
			}
			else if (auto rayNode = ray2->GetNode()) {
				rayNode->SetVisible(false);
			}
		}

		if (auto ray3 = _ray3.Get()) {
			PlaceRay(ray3, ballHit->_targetCenter, collisionNormal, static_cast<float>(_maxRayLength));
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
