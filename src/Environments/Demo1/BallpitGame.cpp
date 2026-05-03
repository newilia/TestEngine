#include "BallpitGame.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

using std::make_shared;
using std::shared_ptr;

namespace Demo1 {

	namespace {

		/// Отдельная маска от Pong (биты 0–1), чтобы шарики не взаимодействовали с чужой сценой.
		constexpr int kAquariumPhysicsGroup = 2;

		constexpr float kBallFriction = 0.12f;

		std::uint8_t PerturbChannel(std::uint8_t base, float variability01, std::mt19937& gen) {
			std::uniform_real_distribution<float> u(-1.f, 1.f);
			const float delta = u(gen) * variability01 * 255.f;
			const float v = static_cast<float>(base) + delta;
			return static_cast<std::uint8_t>(std::clamp(v, 0.f, 255.f));
		}

		sf::Color RandomBallColor(sf::Color base, float colorVariability, std::mt19937& gen) {
			return {PerturbChannel(base.r, colorVariability, gen), PerturbChannel(base.g, colorVariability, gen),
			        PerturbChannel(base.b, colorVariability, gen), PerturbChannel(base.a, colorVariability, gen)};
		}

		void AddAquariumWalls(SceneNode* root, float innerW, float innerH, float wallT, float wallRestitution) {
			const float hx = innerW * 0.5f;
			const float hy = innerH * 0.5f;
			const float t = wallT;

			const sf::Vector2f horizSize{innerW + 2.f * t, t};
			const sf::Vector2f vertSize{t, innerH + 2.f * t};

			const char* wallNames[] = {"Ballpit_bottom", "Ballpit_top", "Ballpit_left", "Ballpit_right"};
			const sf::Vector2f wallSizes[] = {horizSize, horizSize, vertSize, vertSize};
			const sf::Vector2f wallCentersLocal[] = {
			    {0.f, hy + t * 0.5f}, {0.f, -hy - t * 0.5f}, {-hx - t * 0.5f, 0.f}, {hx + t * 0.5f, 0.f}};

			for (int i = 0; i < 4; ++i) {
				auto wallNode = make_shared<SceneNode>();
				wallNode->SetName(wallNames[i]);
				auto rectVisual = make_shared<RectangleShapeVisual>();
				wallNode->SetVisual(rectVisual);
				auto* rectShape = rectVisual->GetShape();
				rectShape->setSize(wallSizes[i]);
				rectShape->setOrigin(Utils::FindCenterOfMass(rectShape));
				rectShape->setFillColor(sf::Color(120, 180, 220, 90));

				root->AddChild(wallNode);
				wallNode->GetLocalTransform()->SetPosition(wallCentersLocal[i]);

				auto bodyBeh = wallNode->RequireBehaviour<PhysicsBodyBehaviour>();
				bodyBeh->SetImmovable();
				bodyBeh->SetRestitution(wallRestitution);
				bodyBeh->GetCollisionGroups().set(kAquariumPhysicsGroup, true);
			}
		}

		bool TryNonOverlappingPosition(const std::vector<std::pair<sf::Vector2f, float>>& placed, sf::Vector2f p,
		                               float r) {
			for (const auto& [q, rq] : placed) {
				if (Utils::Length(p - q) < r + rq - 0.25f) {
					return false;
				}
			}
			return true;
		}

		void AddBalls(SceneNode* root, float innerW, float innerH, float baseR, float radiusVar, sf::Color baseColor,
		              float colorVar, int ballCount, float ballRestitution, std::mt19937& gen) {
			const float hx = innerW * 0.5f;
			const float hy = innerH * 0.5f;
			std::uniform_real_distribution<float> radiusOff(-radiusVar, radiusVar);
			std::uniform_real_distribution<float> velXY(-120.f, 120.f);

			std::vector<std::pair<sf::Vector2f, float>> placed;
			placed.reserve(static_cast<std::size_t>(ballCount));

			for (int i = 0; i < ballCount; ++i) {
				float r = std::max(2.f, baseR + radiusOff(gen));
				r = std::min(r, std::min(hx, hy) - 4.f);
				if (r < 2.f) {
					r = 2.f;
				}

				sf::Vector2f pos{};
				bool ok = false;
				for (int attempt = 0; attempt < 48; ++attempt) {
					std::uniform_real_distribution<float> dx(-hx + r, hx - r);
					std::uniform_real_distribution<float> dy(-hy + r, hy - r);
					const sf::Vector2f trial{dx(gen), dy(gen)};
					if (TryNonOverlappingPosition(placed, trial, r)) {
						pos = trial;
						ok = true;
						break;
					}
				}
				if (!ok) {
					std::uniform_real_distribution<float> dx(-hx + r, hx - r);
					std::uniform_real_distribution<float> dy(-hy + r, hy - r);
					pos = {dx(gen), dy(gen)};
				}
				placed.emplace_back(pos, r);

				auto ballNode = make_shared<SceneNode>();
				ballNode->SetName("Ballpit_ball");
				auto circleVisual = make_shared<CircleShapeVisual>();
				ballNode->SetVisual(circleVisual);
				auto* shape = circleVisual->GetShape();
				shape->setPointCount(32);
				shape->setRadius(r);
				const auto pointCount = static_cast<unsigned>(std::max(8.f, 3.f * (7.f + r * (1.f / 8.f))));
				shape->setPointCount(pointCount);

				const sf::Color fill = RandomBallColor(baseColor, colorVar, gen);
				shape->setFillColor(fill);
				sf::Color outline = fill;
				outline.a = 255;
				shape->setOutlineColor(outline);
				shape->setOutlineThickness(1);
				shape->setOrigin(Utils::FindCenterOfMass(shape));

				auto rb = ballNode->RequireBehaviour<PhysicsBodyBehaviour>();
				rb->SetMass(3.14159265f * r * r);
				rb->SetRestitution(ballRestitution);
				rb->SetFriction(kBallFriction);
				rb->SetVelocity({velXY(gen), velXY(gen)});
				rb->GetCollisionGroups().set(kAquariumPhysicsGroup, true);

				root->AddChild(ballNode);
				ballNode->GetLocalTransform()->SetPosition(pos);
			}
		}

	} // namespace

	shared_ptr<SceneNode> CreateBallpitGameNode(float aquariumWidth, float aquariumHeight, float wallThickness,
	                                            float baseBallRadius, float ballRadiusVariability,
	                                            sf::Color baseBallColor, float ballColorVariability, int ballCount,
	                                            float ballRestitution, float wallRestitution) {
		if (aquariumWidth <= 0.f || aquariumHeight <= 0.f || wallThickness <= 0.f || baseBallRadius <= 0.f ||
		    ballRadiusVariability < 0.f || ballColorVariability < 0.f || ballCount <= 0 || ballRestitution < 0.f ||
		    wallRestitution < 0.f) {
			return nullptr;
		}
		auto* window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return nullptr;
		}

		const float maxR = baseBallRadius + ballRadiusVariability;
		if (maxR * 2.f >= std::min(aquariumWidth, aquariumHeight) - 2.f) {
			return nullptr;
		}

		auto root = make_shared<SceneNode>();
		root->SetName("Ballpit");

		AddAquariumWalls(root.get(), aquariumWidth, aquariumHeight, wallThickness, wallRestitution);

		std::random_device rd;
		std::mt19937 gen(rd());
		AddBalls(root.get(), aquariumWidth, aquariumHeight, baseBallRadius, ballRadiusVariability, baseBallColor,
		         ballColorVariability, ballCount, ballRestitution, gen);

		return root;
	}

} // namespace Demo1
