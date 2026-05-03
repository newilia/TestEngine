#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/Signal.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <bitset>
#include <cstddef>
#include <limits>
#include <memory>
#include <variant>

struct IntersectionDetails;
class SceneNode;

class PhysicsBodyBehaviour : public Behaviour
{
	META_CLASS()
public:
	static constexpr int CollisionGroupsCount = 8;
	using CollisionGroups = std::bitset<PhysicsBodyBehaviour::CollisionGroupsCount>;

	PhysicsBodyBehaviour();
	~PhysicsBodyBehaviour() override;

	void OnInit() override;
	void OnDeinit() override;

	sf::Shape* GetShape();
	const sf::Shape* GetShape() const;
	sf::FloatRect GetBbox() const;
	size_t GetPointCount() const;
	sf::Vector2f GetPointGlobal(std::size_t index) const;
	void SetImmovable();
	bool IsImmovable() const;
	float GetMass() const;
	void SetMass(float m);
	sf::Vector2f GetVelocity() const;
	void SetVelocity(sf::Vector2f v);
	void AddVelocity(sf::Vector2f delta);
	void ScaleVelocity(float factor);
	float GetAngle() const;
	void SetAngle(float a);
	float GetAngularSpeed() const;
	void SetAngularSpeed(float w);
	float GetRestitution() const;
	void SetRestitution(float r);
	float GetFriction() const;
	void SetFriction(float f);
	float GetGravityScale() const;
	void SetGravityScale(float s);
	CollisionGroups& GetCollisionGroups();
	const CollisionGroups& GetCollisionGroups() const;
	CollisionGroups& GetOverlappingGroups();
	const CollisionGroups& GetOverlappingGroups() const;
	Signal<const IntersectionDetails&>& GetCollisionCallbacks();
	Signal<const IntersectionDetails&>& GetOverlappingCallbacks();

private:
	/// @property
	float _mass = 1.f;
	/// @property
	sf::Vector2f _velocity{};
	/// @property(name="Angle (rad)")
	float _angle = 0.f;
	/// @property
	float _angularSpeed = 0.f;
	/// @property
	float _restitution = 0.5f;
	/// @property
	float _friction = 0.5f;
	/// @property(minValue=-1.f, maxValue=1.f, tooltip="Scales world gravity on this body. 0 = none; -1 = opposite.")
	float _gravityScale = 1.f;
	/// @property
	CollisionGroups _collisionGroups;
	/// @property(readonly=true)
	CollisionGroups _overlappingGroups;

	Signal<const IntersectionDetails&> _collisionCallbacks;
	Signal<const IntersectionDetails&> _overlappingCallbacks;
};
