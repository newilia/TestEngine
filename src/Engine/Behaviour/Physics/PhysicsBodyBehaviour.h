#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <variant>

struct IntersectionDetails;
class SceneNode;

class PhysicsBodyBehaviour : public Behaviour, public std::enable_shared_from_this<PhysicsBodyBehaviour>
{
	META_CLASS()

public:
	static constexpr int kGroupsCount = 8;
	using GroupSet = std::bitset<PhysicsBodyBehaviour::kGroupsCount>;

	PhysicsBodyBehaviour() = default;

public:
	void OnInit() override;
	void OnDeinit() override;
	void OnEnabled(bool isEnabled) override;

	const sf::Shape* GetColliderShape() const;

	bool IsFixed() const;
	void SetFixed(bool isFixed);

	float GetMass() const;
	void SetMass(float m);

	sf::Vector2f GetVelocity() const;
	void SetVelocity(sf::Vector2f v);
	void AddVelocity(sf::Vector2f delta);

	float GetAngularSpeed() const;
	void SetAngularSpeed(float w);

	float GetRestitution() const;
	void SetRestitution(float r);

	float GetFriction() const;
	void SetFriction(float f);

	float GetGravityScale() const;
	void SetGravityScale(float s);

	GroupSet& GetCollisionGroups();
	const GroupSet& GetCollisionGroups() const;

	GroupSet& GetOverlapGroups();
	const GroupSet& GetOverlapGroups() const;

	Signal<const IntersectionDetails&>& GetOnCollideSignal() const;
	Signal<const IntersectionDetails&>& GetOnOverlapSignal() const;

	/// Rebuilds cached collider COM / inertia geometry when the shape instance changes.
	void RefreshCollisionShapeCache();
	[[nodiscard]] sf::Vector2f GetCollisionComWorld(const sf::Transform& shapeToWorld) const;
	[[nodiscard]] float EstimateCollisionInertiaWorld(float mass, const sf::Transform& shapeToWorld) const;

private:
	/// @property
	float _mass = 1.f;
	/// @property
	bool _isFixed = false;
	/// @property
	sf::Vector2f _velocity{};
	/// @property
	float _angularSpeed = 0.f;
	/// @property(minValue=0.f, maxValue=1.f, dragSpeed=0.05f)
	float _restitution = 0.5f;
	/// @property(minValue=0.f, maxValue=1.f, dragSpeed=0.05f)
	float _friction = 0.5f;
	/// @property(minValue=-1.f, maxValue=1.f, dragSpeed=0.05f, tooltip="Scales world gravity on this body")
	float _gravityScale = 1.f;
	/// @property
	GroupSet _collisionGroups = {{true}};
	/// @property(tooltip="Bodies with common groups will trigger overlap signal, but won't interact in a physical way")
	GroupSet _overlapGroups;

	mutable Signal<const IntersectionDetails&> _onCollideSignal;
	mutable Signal<const IntersectionDetails&> _onOverlapSignal;

	enum class CollisionGeomKind : std::uint8_t
	{
		None,
		Circle,
		Rectangle,
		PolygonRough
	};

	static float MinCollisionInertia(float mass);

	const sf::Shape* _collisionCacheShapePtr = nullptr;
	bool _collisionGeomCacheOk = false;
	CollisionGeomKind _collisionGeomKind = CollisionGeomKind::None;
	sf::Vector2f _collisionComLocal{};
	float _collisionCircleRadiusLocal = 0.f;
	sf::Vector2f _collisionRectSize{};
	/// I_z / m in shape-local units ([length]²), convex polygon via fan from vertex 0 + parallel axis to COM.
	float _polygonIzPerMassLocal = 0.f;

	mutable bool _inertiaWorldCacheValid = false;
	mutable float _cachedInertiaWorld = 0.f;
	mutable float _cachedInertiaMass = 0.f;
	mutable float _cachedInertiaShapeToWorldMatrix[16]{};

	[[nodiscard]] bool InertiaWorldCacheMatches(float mass, const sf::Transform& shapeToWorld) const;
	void StoreInertiaWorldCache(float mass, const sf::Transform& shapeToWorld, float inertiaWorld) const;
};
