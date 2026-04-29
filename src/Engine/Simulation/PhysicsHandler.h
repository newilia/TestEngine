#pragma once

#include "Engine/Behaviour/Physics/AbstractBody.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Updatable.h"
#include "Engine/Core/common.h"
#include "IsotropicInverseSquareField.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <list>
#include <memory>
#include <optional>

class PhysicsHandler : public Updatable
{
public:
	~PhysicsHandler() override = default;
	void Update(const sf::Time& dt) override;
	void RegisterBody(shared_ptr<SceneNode> body);
	void UnregisterBody(SceneNode* body);

	const auto& GetAllBodies() { return _bodies; }

	void SetSubstepCount(int count) { _subStepsCount = count; }

	[[nodiscard]] int GetSubstepCount() const { return _subStepsCount; }

	void SetGravity(const sf::Vector2f v) { _gravity = v; }

	sf::Vector2f GetGravity() const { return _gravity; }

	void SetGravityEnabled(bool enabled) { _isGravityEnabled = enabled; }

	bool IsGravityEnabled() const { return _isGravityEnabled; }

	[[nodiscard]] std::shared_ptr<IsotropicInverseSquareField> GetIsotropicInverseSquareField() const {
		return _inverseSquareField;
	}

private:
	void UpdateSubStep(const sf::Time& dt);
	static bool CheckBboxIntersection(const AbstractBody* body1, const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectIntersection(const shared_ptr<SceneNode>& n1,
	                                                             const shared_ptr<SceneNode>& n2);
	static std::optional<IntersectionDetails> DetectPolygonPolygonIntersection(const AbstractBody* body1,
	                                                                           const AbstractBody* body2);
	static std::optional<IntersectionDetails> DetectCirclePolygonIntersection(const sf::CircleShape* circle,
	                                                                          const AbstractBody* polygon);
	static std::optional<IntersectionDetails> DetectCircleCircleIntersection(const sf::CircleShape* circle1,
	                                                                         const sf::CircleShape* circle2);
	static std::optional<SegmentIntersectionPoints> FindSegmentsIntersectionPoint(const Segment& e, const Segment& f);
	static std::optional<SegmentIntersectionPoints>
	FindSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius);
	static void ResolveCollision(const IntersectionDetails& collision);
	std::list<std::weak_ptr<SceneNode>> _bodies;
	int _subStepsCount = 1;
	sf::Vector2f _gravity = {0, 400};
	bool _isGravityEnabled = false;
	std::shared_ptr<IsotropicInverseSquareField> _inverseSquareField = std::make_shared<IsotropicInverseSquareField>();
};
