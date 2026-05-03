#include "SolarSystemBehaviour.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/AttractionField.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "SolarSystemBehaviour.generated.hpp"

#include <SFML/Graphics/CircleShape.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>

namespace {

	constexpr int kPlanetCount = 8;

	struct PlanetDef
	{
		const char* name;
		sf::Color fill;
		sf::Color outline;
		float semiMajorAxisAu;
		float eccentricity;
		float massEarthMasses;
		float radiusEarthRadii;
		float meanOrbitalSpeedKmS;
	};

	// Semi-major axis (AU), e, mass (Earth=1), radius (Earth=1), mean orbital speed (km/s) — order Mercury→Neptune.
	const std::array<PlanetDef, kPlanetCount> kPlanets = {{
	    {"Mercury", {180, 170, 150, 255}, {120, 110, 90, 255}, 0.387f, 0.206f, 0.055f, 0.383f, 47.4f},
	    {"Venus", {230, 210, 140, 255}, {180, 160, 100, 255}, 0.723f, 0.007f, 0.815f, 0.949f, 35.0f},
	    {"Earth", {60, 120, 200, 255}, {30, 80, 160, 255}, 1.0f, 0.017f, 1.0f, 1.0f, 29.78f},
	    {"Mars", {200, 90, 60, 255}, {160, 50, 30, 255}, 1.524f, 0.0934f, 0.107f, 0.532f, 24.07f},
	    {"Jupiter", {210, 170, 120, 255}, {160, 120, 70, 255}, 5.204f, 0.0489f, 317.8f, 11.2f, 13.07f},
	    {"Saturn", {220, 200, 160, 255}, {180, 160, 120, 255}, 9.5826f, 0.0565f, 95.2f, 9.45f, 9.69f},
	    {"Uranus", {120, 200, 220, 255}, {80, 160, 200, 255}, 19.2184f, 0.0463f, 14.5f, 4.0f, 6.80f},
	    {"Neptune", {80, 100, 200, 255}, {50, 70, 180, 255}, 30.11f, 0.0086f, 17.1f, 3.88f, 5.43f},
	}};

	void SetSolarInteractionMask(PhysicsBodyBehaviour& rb, int planetIndexOrMinusOneForSun) {
		auto& g = rb.GetInteractionGroups();
		g.reset();
		g.set(7, true);
		/*if (planetIndexOrMinusOneForSun < 0) {
			for (int i = 0; i < kPlanetCount; ++i) {
				g.set(static_cast<std::size_t>(i), true);
			}
			return;
		}
		if (planetIndexOrMinusOneForSun >= 0 && planetIndexOrMinusOneForSun < kPlanetCount) {
			g.set(static_cast<std::size_t>(planetIndexOrMinusOneForSun), true);
		}*/
	}

} // namespace

void SolarSystemBehaviour::OnInit() {
	Restart();
}

void SolarSystemBehaviour::OnUpdate(const sf::Time& /*dt*/) {
	SyncSunAttractionFromProperty();
}

void SolarSystemBehaviour::SetSolarSystemRoot(std::weak_ptr<SceneNode> root) {
	_solarSystemRoot = std::move(root);
}

void SolarSystemBehaviour::SyncSunAttractionFromProperty() const {
	if (const auto sun = _sunAttractive.lock()) {
		sun->SetAttraction(_sunAttraction);
	}
}

void SolarSystemBehaviour::Restart() {
	const auto root = _solarSystemRoot.lock();
	if (!root) {
		return;
	}

	_sunAttractive.reset();

	auto kids = root->GetChildren();
	for (const auto& c : kids) {
		root->RemoveChild(c.get());
	}

	const float auToPx = _distanceScale;
	const float vChar = 72.f;

	{
		auto sunNode = std::make_shared<SceneNode>();
		sunNode->SetName("SolarSun");
		auto circleVisual = std::make_shared<CircleShapeVisual>();
		sunNode->SetVisual(circleVisual);
		auto* shape = circleVisual->GetShape();
		const float sunVisR = std::max(8.f, 6.96f * _visualRadiusScale);
		shape->setRadius(sunVisR);
		shape->setPointCount(48);
		shape->setFillColor(sf::Color(255, 220, 60, 255));
		shape->setOutlineColor(sf::Color(255, 180, 40, 255));
		shape->setOutlineThickness(_outlineThickness);
		shape->setOrigin(Utils::FindCenterOfMass(shape));

		auto sunRb = sunNode->RequireBehaviour<PhysicsBodyBehaviour>();
		sunRb->SetImmovable(true);
		sunRb->SetMass(333000.f * _massScale);
		sunRb->SetGravityScale(0.f);
		sunRb->SetRestitution(0.1f);
		sunRb->SetFriction(0.f);
		SetSolarInteractionMask(*sunRb, -1);

		auto attractive = std::make_shared<AttractiveBehaviour>();
		attractive->SetAttraction(_sunAttraction);
		_sunAttractive = attractive;
		sunNode->AddBehaviour(std::move(attractive));

		root->AddChild(sunNode);
		sunNode->GetLocalTransform()->SetPosition({0.f, 0.f});
	}

	for (int i = 0; i < kPlanetCount; ++i) {
		const PlanetDef& p = kPlanets[static_cast<std::size_t>(i)];
		const float aPx = p.semiMajorAxisAu * auToPx;
		const float e = p.eccentricity;
		const float rPeri = aPx * (1.f - e);
		const sf::Vector2f pos{rPeri, 0.f};

		const float r = std::max(1.f, pos.x * pos.x + pos.y * pos.y);
		const sf::Vector2f radial = pos / std::sqrt(r);
		const sf::Vector2f tangent{-radial.y, radial.x};
		const float vModel = vChar * (p.meanOrbitalSpeedKmS / 29.78f);
		const sf::Vector2f vel = tangent * (vModel * _orbitalSpeedScale);

		auto planet = std::make_shared<SceneNode>();
		planet->SetName(std::string("Planet_") + p.name);
		auto circleVisual = std::make_shared<CircleShapeVisual>();
		planet->SetVisual(circleVisual);
		auto* shape = circleVisual->GetShape();
		const float visR = std::max(2.f, p.radiusEarthRadii * _visualRadiusScale);
		shape->setRadius(visR);
		const auto pc = static_cast<unsigned>(std::max(8.f, 3.f * (7.f + visR * (1.f / 8.f))));
		shape->setPointCount(pc);
		shape->setFillColor(p.fill);
		shape->setOutlineColor(p.outline);
		shape->setOutlineThickness(_outlineThickness);
		shape->setOrigin(Utils::FindCenterOfMass(shape));

		auto rb = planet->RequireBehaviour<PhysicsBodyBehaviour>();
		rb->SetMass(std::max(1e-6f, p.massEarthMasses * _massScale));
		rb->SetVelocity(vel);
		rb->SetGravityScale(0.f);
		rb->SetRestitution(0.f);
		rb->SetFriction(0.f);
		SetSolarInteractionMask(*rb, i);

		auto recv = std::make_shared<AttractiveBehaviour>();
		recv->SetAttraction(0.f);
		planet->AddBehaviour(recv);

		root->AddChild(planet);
		planet->GetLocalTransform()->SetPosition(pos);
	}

	root->NotifyLifecycleInitRecursive();
}
