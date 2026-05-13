#pragma once

#include "AttractionField.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Updatable.h"
#include "Engine/Core/common.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <algorithm>
#include <list>
#include <memory>
#include <optional>
#include <random>
#include <utility>
#include <vector>

class PhysicsProcessor : public Updatable
{
public:
	~PhysicsProcessor() override = default;
	void Update(const sf::Time& dt) override;

	void RegisterBody(shared_ptr<PhysicsBodyBehaviour> body);
	void UnregisterBody(PhysicsBodyBehaviour* body);
	const std::list<std::weak_ptr<PhysicsBodyBehaviour>>& GetAllBodies() const;

	void SetGravity(const sf::Vector2f v);
	sf::Vector2f GetGravity() const;

	void SetGravityEnabled(bool enabled);
	bool IsGravityEnabled() const;

	void SetAirFriction(float airFriction);
	float GetAirFriction() const;

	int GetSimulationSubsteps() const;
	void SetSimulationSubsteps(int substeps);

	std::shared_ptr<AttractionField> GetAttractionField() const;

	sf::Vector2f EvaluateExternalForces(PhysicsBodyBehaviour* body) const;

private:
	template <typename Fn, typename URBG>
	void VisitBodiesInRandomOrder(Fn&& visitor, URBG&& urbg);
	template <typename Fn>
	void VisitBodiesInRandomOrder(Fn&& visitor);

	void IntergateVelocity(PhysicsBodyBehaviour* body, float dtSec);
	void IntegratePosition(PhysicsBodyBehaviour* body, float dtSec);
	void DetactAndResolveCollisions();
	void ResolveCollision(const IntersectionDetails& collision);

	static std::optional<IntersectionDetails> DetectIntersection(
	    SceneNode* node1, SceneNode* node2, PhysicsBodyBehaviour* body1, PhysicsBodyBehaviour* body2);
	static std::optional<IntersectionDetails> DetectPolygonPolygonIntersection(
	    const PhysicsBodyBehaviour* body1, const PhysicsBodyBehaviour* body2);
	static std::optional<IntersectionDetails> DetectCirclePolygonIntersection(const SceneNode& circleNode,
	    const sf::CircleShape* circle, const SceneNode& polygonNode, const sf::Shape* polygonShape);
	static std::optional<IntersectionDetails> DetectCircleCircleIntersection(
	    const SceneNode& node1, const sf::CircleShape* circle1, const SceneNode& node2, const sf::CircleShape* circle2);
	static std::optional<SegmentIntersectionPoints> FindSegmentsIntersectionPoint(const Segment& e, const Segment& f);
	static std::optional<SegmentIntersectionPoints> FindSegmentCircleIntersectionPoint(
	    const Segment& seg, const sf::Vector2f& circleCenter, float radius);

private:
	std::shared_ptr<AttractionField> _attractionField = std::make_shared<AttractionField>();
	std::list<std::weak_ptr<PhysicsBodyBehaviour>> _bodies;
	sf::Vector2f _gravity = {0, 400};
	bool _isGravityEnabled = false;
	float _airFriction = 0.f;
	int _simulationSubsteps = 1;
};

template <typename Fn, typename URBG>
void PhysicsProcessor::VisitBodiesInRandomOrder(Fn&& visitor, URBG&& urbg) {
	if (_bodies.empty()) {
		return;
	}
	std::vector<std::weak_ptr<PhysicsBodyBehaviour>> order;
	order.reserve(_bodies.size());
	for (const auto& w : _bodies) {
		order.push_back(w);
	}
	std::shuffle(order.begin(), order.end(), std::forward<URBG>(urbg));
	for (auto& w : order) {
		std::forward<Fn>(visitor)(w);
	}
}

template <typename Fn>
void PhysicsProcessor::VisitBodiesInRandomOrder(Fn&& visitor) {
	thread_local std::mt19937 rng(std::random_device{}());
	VisitBodiesInRandomOrder(std::forward<Fn>(visitor), rng);
}
