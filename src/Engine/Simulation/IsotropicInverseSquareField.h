#pragma once

#include "Engine/Core/common.h"

#include <SFML/System/Vector2.hpp>

#include <list>
#include <memory>

class InverseSquareFieldSourceBehaviour;

/// Superposition of isotropic central \(1/r^2\)-type (Newtonian / Coulomb) interactions in 2D.
class IsotropicInverseSquareField
{
public:
	/// @param s Must be the behaviour that calls Register() from OnInit; stored as weak ref.
	void Register(const shared_ptr<InverseSquareFieldSourceBehaviour>& s);
	void Unregister(const shared_ptr<InverseSquareFieldSourceBehaviour>& s);

	/// Net field acceleration at the receiving behaviour's world position, due to all **other** registered sources.
	/// Does not include self; respects each source's `_isEnabled` and `EvaluateAcceleration` caches purge expired.
	[[nodiscard]] sf::Vector2f
	EvaluateAcceleration(const shared_ptr<InverseSquareFieldSourceBehaviour>& receiver) const;

	[[nodiscard]] float GetGlobalStrengthScale() const;
	void SetGlobalStrengthScale(float v);

	[[nodiscard]] bool GetUseMassCoupling() const;
	void SetUseMassCoupling(bool v);

	[[nodiscard]] float GetSofteningEps() const;
	void SetSofteningEps(float e);

private:
	mutable std::list<std::weak_ptr<InverseSquareFieldSourceBehaviour>> _sources;
	float _globalStrengthScale = 50.f;
	bool _useMassCoupling = true;
	mutable float _softeningEps = 4.f;
};
