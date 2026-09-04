#include "BilliardBallSoundEventsBehaviour.h"

#include "BilliardBallBehaviour.h"
#include "BilliardBallSoundEventsBehaviour.generated.hpp"
#include "BilliardCueBehaviour.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <cmath>
#include <limits>
#include <utility>

namespace Billiard {

	namespace {
		constexpr float kMinApproachSpeed = 0.1f;
		constexpr std::string_view kBallBallCollisionEvent = "event:/SFX/ball_ball_collision";
		constexpr std::string_view kBallRailCollisionEvent = "event:/SFX/ball_rail_collision";
		constexpr std::string_view kCueBallCollisionEvent = "event:/SFX/cue_ball_collision";
		constexpr std::string_view kIntensityParameterName = "Intensity";

		void PlaySoundEvent(std::string_view eventPath, float speed) {
			auto audio = Engine::MainContext::GetInstance().GetAudioManager();
			if (!audio || !audio->IsInitialized()) {
				return;
			}
			const Engine::EventParameter parameters[] = {{kIntensityParameterName, speed}};
			audio->PlayEvent(eventPath, parameters);
		}

		[[nodiscard]] sf::Vector2f GetSeparationReferencePoint(
		    const SceneNode& node, const PhysicsBodyBehaviour& body) {
			const sf::Shape* shape = body.GetColliderShape();
			assert(shape);
			const sf::Transform shapeToWorld = node.GetWorldTransform() * shape->getTransform();
			if (const auto* circle = dynamic_cast<const sf::CircleShape*>(shape)) {
				return shapeToWorld.transformPoint(circle->getGeometricCenter());
			}
			return body.GetCollisionComWorld(shapeToWorld);
		}

		[[nodiscard]] float GetApproachSpeed(const IntersectionDetails& intersection) {
			const auto node1 = intersection.wNode1.lock();
			const auto node2 = intersection.wNode2.lock();
			if (!node1 || !node2) {
				return 0.f;
			}

			const auto body1 = node1->FindBehaviour<PhysicsBodyBehaviour>();
			const auto body2 = node2->FindBehaviour<PhysicsBodyBehaviour>();
			if (!body1 || !body2) {
				return 0.f;
			}

			const sf::Vector2f delta =
			    GetSeparationReferencePoint(*node2, *body2) - GetSeparationReferencePoint(*node1, *body1);
			const float distSq = delta.x * delta.x + delta.y * delta.y;
			if (distSq <= std::numeric_limits<float>::epsilon()) {
				return 0.f;
			}

			const sf::Vector2f separationDir = delta / std::sqrt(distSq);
			const sf::Vector2f relativeVelocity = body2->GetVelocity() - body1->GetVelocity();
			return std::max(0.f, Utils::Dot(relativeVelocity, separationDir));
		}
	} // namespace

	void BilliardBallSoundEventsBehaviour::OnInit() {
		Behaviour::OnInit();

		const auto node = GetNode();
		if (!node) {
			return;
		}

		const auto physicsBody = node->FindBehaviour<PhysicsBodyBehaviour>();
		if (!physicsBody) {
			return;
		}

		Subscribe(physicsBody->GetOnCollideSignal(), [this](const IntersectionDetails& intersection) {
			OnPhysicsContact(intersection, false);
		});
		Subscribe(physicsBody->GetOnOverlapSignal(), [this](const IntersectionDetails& intersection) {
			OnPhysicsContact(intersection, true);
		});
	}

	void BilliardBallSoundEventsBehaviour::OnDeinit() {
		UnsubscribeAll();
		Behaviour::OnDeinit();
	}

	void BilliardBallSoundEventsBehaviour::OnPhysicsContact(const IntersectionDetails& intersection, bool isOverlap) {
		const auto selfNode = GetNode();
		auto node1 = intersection.wNode1.lock();
		auto node2 = intersection.wNode2.lock();
		if (!selfNode || !node1 || !node2) {
			return;
		}

		const auto otherNode = (node1 == selfNode) ? node2 : node1;
		if (!otherNode) {
			return;
		}

		const float approachSpeed = GetApproachSpeed(intersection);
		if (approachSpeed < kMinApproachSpeed) {
			return;
		}

		if (otherNode->FindBehaviour<BilliardCueBehaviour>()) {
			PlaySoundEvent(kCueBallCollisionEvent, approachSpeed);
			return;
		}

		if (isOverlap) {
			return;
		}

		if (otherNode->FindBehaviour<BilliardBallBehaviour>()) {
			PlaySoundEvent(kBallBallCollisionEvent, approachSpeed);
			return;
		}

		if (otherNode->GetVisual<RectangleShapeVisual>()) {
			PlaySoundEvent(kBallRailCollisionEvent, approachSpeed);
		}
	}

} // namespace Billiard
