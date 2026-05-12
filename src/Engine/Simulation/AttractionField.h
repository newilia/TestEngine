#pragma once

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/common.h"

#include <SFML/System/Vector2.hpp>

#include <list>
#include <memory>

class AttractiveBehaviour;

/// Superposition of isotropic central \(1/r^2\)-type (Newtonian / Coulomb) interactions in 2D.
class AttractionField
{
public:
	/// @param s Must be the behaviour that calls Register() from OnInit; stored as weak ref.
	void Register(const shared_ptr<AttractiveBehaviour>& s);
	void Unregister(const shared_ptr<AttractiveBehaviour>& s);

	/// Net field at `worldPos` from all enabled sources that interact with `interactionGroups` (no receiver exclusion).
	sf::Vector2f EvaluateForce(sf::Vector2f worldPos, const PhysicsBodyBehaviour::GroupSet& interactionGroups) const;

	/// Net field at the receiver's world position from all **other** registered sources (excludes self).
	sf::Vector2f EvaluateForce(const shared_ptr<AttractiveBehaviour>& receiver) const;

	float GetGlobalStrengthScale() const;
	void SetGlobalStrengthScale(float v);

	bool GetUseMassCoupling() const;
	void SetUseMassCoupling(bool v);

	float GetSofteningEps() const;
	void SetSofteningEps(float e);

private:
	sf::Vector2f EvaluateForceImpl(sf::Vector2f worldPos, const PhysicsBodyBehaviour::GroupSet& interactionGroups,
	    const AttractiveBehaviour* excludeSource) const;

	mutable std::list<std::weak_ptr<AttractiveBehaviour>> _sources;
	float _globalStrengthScale = 50.f;
	bool _useMassCoupling = true;
	mutable float _softeningEps = 4.f;
};
