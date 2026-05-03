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
	static constexpr int kGroupsCount = 8;
	using GroupSet = std::bitset<PhysicsBodyBehaviour::kGroupsCount>;

	PhysicsBodyBehaviour();
	~PhysicsBodyBehaviour() override;

	void OnInit() override;
	void OnDeinit() override;

	sf::Shape* GetShape();
	const sf::Shape* GetShape() const;

	sf::FloatRect GetBbox() const;

	size_t GetPointCount() const;
	sf::Vector2f GetPointWorldPos(std::size_t pointIndex) const;

	bool IsImmovable() const;
	void SetImmovable(bool isImmovable);

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

	GroupSet& GetInteractionGroups();
	const GroupSet& GetInteractionGroups() const;

	GroupSet& GetOverlappingGroups();
	const GroupSet& GetOverlappingGroups() const;

	Signal<const IntersectionDetails&>& GetOnCollideSignal();
	Signal<const IntersectionDetails&>& GetOnOverlapSignal();

private:
	/// @property
	float _mass = 1.f;
	/// @property
	sf::Vector2f _velocity{};
	/// @property(tooltip="Radians")
	float _angle = 0.f;
	/// @property
	float _angularSpeed = 0.f;
	/// @property(minValue=0.f, maxValue=1.f)
	float _restitution = 0.5f;
	/// @property
	float _friction = 0.5f;
	/// @property(minValue=-1.f, maxValue=1.f, dragSpeed=0.05f, tooltip="Scales world gravity on this body")
	float _gravityScale = 1.f;
	/// @property(tooltip="Groups with common groups will interact in a physical way (collisions, forces, etc.)")
	GroupSet _interactionGroups;
	/// @property(tooltip="Groups with common groups will trigger overlap events, but won't interact in a physical way")
	GroupSet _overlappingGroups;

	Signal<const IntersectionDetails&> _collisionCallbacks;
	Signal<const IntersectionDetails&> _overlappingCallbacks;
};
