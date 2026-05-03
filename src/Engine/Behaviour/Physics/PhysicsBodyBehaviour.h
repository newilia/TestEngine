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

	PhysicsBodyBehaviour();
	~PhysicsBodyBehaviour() override;

	void OnInit() override;
	void OnDeinit() override;

	sf::Shape* GetShape();
	const sf::Shape* GetShape() const;

	sf::FloatRect GetBbox() const;
	size_t GetPointCount() const;
	sf::Vector2f GetPointGlobal(std::size_t index) const;
	sf::Vector2f GetPosGlobal() const;
	void SetPosGlobal(sf::Vector2f pos);

	void SetImmovable();
	bool IsImmovable() const;

	float GetMass() const {
		return _mass;
	}

	void SetMass(float m) {
		_mass = m;
	}

	sf::Vector2f GetVelocity() const {
		return _velocity;
	}

	void SetVelocity(sf::Vector2f v) {
		_velocity = v;
	}

	void AddVelocity(sf::Vector2f delta) {
		_velocity += delta;
	}

	void ScaleVelocity(float factor) {
		_velocity *= factor;
	}

	float GetAngle() const {
		return _angle;
	}

	void SetAngle(float a) {
		_angle = a;
	}

	float GetAngularSpeed() const {
		return _angularSpeed;
	}

	void SetAngularSpeed(float w) {
		_angularSpeed = w;
	}

	float GetRestitution() const {
		return _restitution;
	}

	void SetRestitution(float r) {
		_restitution = r;
	}

	float GetFriction() const {
		return _friction;
	}

	void SetFriction(float f) {
		_friction = f;
	}

	float GetGravityScale() const {
		return _gravityScale;
	}

	void SetGravityScale(float s) {
		_gravityScale = s;
	}

	std::bitset<CollisionGroupsCount>& GetCollisionGroups() {
		return _collisionGroups;
	}

	const std::bitset<CollisionGroupsCount>& GetCollisionGroups() const {
		return _collisionGroups;
	}

	std::bitset<CollisionGroupsCount>& GetOverlappingGroups() {
		return _overlappingGroups;
	}

	const std::bitset<CollisionGroupsCount>& GetOverlappingGroups() const {
		return _overlappingGroups;
	}

	Signal<const IntersectionDetails&>& GetCollisionCallbacks() {
		return _collisionCallbacks;
	}

	Signal<const IntersectionDetails&>& GetOverlappingCallbacks() {
		return _overlappingCallbacks;
	}

private:
	/// @property(tooltip="Infinity = immovable; use SetImmovable() in code, or set mass in inspector.")
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

	std::bitset<CollisionGroupsCount> _collisionGroups;
	std::bitset<CollisionGroupsCount> _overlappingGroups;

	Signal<const IntersectionDetails&> _collisionCallbacks;
	Signal<const IntersectionDetails&> _overlappingCallbacks;

	/// @property(readonly=true, tooltip="Set when body is registered with the physics handler.")
	bool _registered = false;
};
