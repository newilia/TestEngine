#include "BilliardBallSpawnBehaviour.h"

#include "BilliardBallSpawnBehaviour.generated.hpp"
#include "Engine/Behaviour/ComposedSurface/SphereOrientation.h"
#include "Engine/Behaviour/ComposedSurface/SphereProjectionContributorBehaviour.h"
#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeClone.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "LaunchProfiles/Billiard/TopDownShadowBehaviour.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <vector>

namespace {

	[[nodiscard]] sf::FloatRect GetRectangleNodeWorldAabb(
	    const SceneNode& node, const RectangleShapeVisual& rectVisual) {
		const auto* shape = dynamic_cast<const sf::RectangleShape*>(rectVisual.GetBaseShape());
		if (!shape) {
			return {};
		}
		sf::Transform transform = node.GetWorldTransform();
		transform *= shape->getTransform();
		return transform.transformRect(shape->getLocalBounds());
	}

	[[nodiscard]] sf::Vector2f RackSlotLocalOffset(int slotIndex, float ballRadius) {
		int row = 1;
		int slotInRow = slotIndex;
		while (slotInRow >= row) {
			slotInRow -= row;
			++row;
		}
		const int col = slotInRow;
		const float rowSpacing = std::sqrt(3.f) * ballRadius;
		const float x = static_cast<float>(row - 1) * rowSpacing;
		const float y = (static_cast<float>(col) - static_cast<float>(row - 1) * 0.5f) * 2.f * ballRadius;
		return {x, y};
	}

} // namespace

void Billiard::BilliardBallSpawnBehaviour::Setup() {
	const auto ballPrefab = _ballPrefab.Get();
	const auto tableRect = _tableRectRef.Get();
	const auto ballParent = _ballParent.Get();
	if (!ballPrefab || !tableRect || !ballParent || _ballRadius <= 0.f) {
		return;
	}

	auto children = ballParent->GetChildren();
	for (auto child : children) {
		ballParent->RemoveChild(child.get());
	}

	const sf::FloatRect tableBounds = GetTableWorldBounds();
	if (tableBounds.size.x <= 0.f || tableBounds.size.y <= 0.f) {
		return;
	}

	const sf::Vector2f leftHalfCenter = {
	    tableBounds.position.x + tableBounds.size.x * 0.25f,
	    tableBounds.position.y + tableBounds.size.y * 0.5f,
	};
	const sf::Vector2f rightHalfCenter = {
	    tableBounds.position.x + tableBounds.size.x * 0.75f,
	    tableBounds.position.y + tableBounds.size.y * 0.5f,
	};

	SpawnBall(0, leftHalfCenter);

	const std::array<int, 15> rackNumbers = BuildRackBallNumbers();
	for (int slot = 0; slot < 15; ++slot) {
		const sf::Vector2f worldPos = rightHalfCenter + RackSlotLocalOffset(slot, _ballRadius);
		SpawnBall(rackNumbers[static_cast<size_t>(slot)], worldPos);
	}
}

void Billiard::BilliardBallSpawnBehaviour::SpawnBall(int ballIndex, sf::Vector2f worldPos) {
	const auto ballPrefab = _ballPrefab.Get();
	const auto parentNode = _ballParent.Get();
	if (!ballPrefab || !parentNode) {
		return;
	}

	const auto instance = Engine::CloneSceneNode(ballPrefab->GetNode());
	if (!instance) {
		return;
	}

	parentNode->AddChild(instance);
	instance->SetName(fmt::format("Ball {}", ballIndex));
	Utils::SetLocalPosToWorld(instance, worldPos);

	if (auto tiledTexture = instance->FindBehaviourRec<Engine::TiledTextureContributorBehaviour>()) {
		tiledTexture->SetTexturePath(FormatTexturePath(ballIndex));
	}

	if (!_lightSources.empty()) {
		SetupShadows(*instance);
	}

	if (auto projection = instance->FindBehaviourRec<Engine::SphereProjectionContributorBehaviour>()) {
		std::random_device randomDevice;
		std::mt19937 rng(randomDevice());
		constexpr float PI = 3.1415;
		auto yaw = std::uniform_real_distribution<float>(-PI, PI)(rng);
		auto pitch = std::uniform_real_distribution<float>(-PI, PI)(rng);
		auto roll = std::uniform_real_distribution<float>(-PI, PI)(rng);
		Engine::SphereOrientationQuat orientation = Engine::SphereOrientationFromEulerYxz(yaw, pitch, roll);
		projection->SetSphereOrientation(orientation);
	}
}

void Billiard::BilliardBallSpawnBehaviour::SetupShadows(SceneNode& ballNode) {
	if (auto shadow = ballNode.FindBehaviourRec<Billiard::TopDownShadowBehaviour>()) {
		const auto shadowNode = shadow->GetNode();
		const auto shadowParent = shadowNode ? shadowNode->GetParent() : nullptr;
		if (shadowNode && shadowParent) {
			RefWrapper<SceneNode> objectRef;
			objectRef.SetId(ballNode.GetEntityId());

			for (std::size_t lightIndex = 0; lightIndex < _lightSources.size(); ++lightIndex) {
				std::shared_ptr<SceneNode> currentShadowNode;
				Billiard::TopDownShadowBehaviour* shadowBehaviour = nullptr;

				if (lightIndex == 0) {
					currentShadowNode = shadowNode;
					shadowBehaviour = shadow.get();
				}
				else {
					currentShadowNode = Engine::CloneSceneNode(shadowNode);
					if (!currentShadowNode) {
						continue;
					}
					shadowParent->AddChild(currentShadowNode);
					currentShadowNode->NotifyLifecycleInitRecursive();
					if (auto clonedShadow = currentShadowNode->FindBehaviour<Billiard::TopDownShadowBehaviour>()) {
						shadowBehaviour = clonedShadow.get();
					}
				}

				if (!shadowBehaviour) {
					continue;
				}

				shadowBehaviour->SetObject(objectRef);
				if (const auto lightNode = _lightSources[lightIndex].Get()) {
					shadowBehaviour->SetLightSource(lightNode);
				}
				shadowBehaviour->UpdateShadowPosition();
			}
		}
	}
}

std::string Billiard::BilliardBallSpawnBehaviour::FormatTexturePath(int ballIndex) const {
	const std::string idPart = fmt::format("{}", ballIndex);
	const std::size_t placeholder = _texturePathMask.find("{}");
	if (placeholder == std::string::npos) {
		return _texturePathMask;
	}
	std::string result = _texturePathMask;
	result.replace(placeholder, 2, idPart);
	return result;
}

sf::FloatRect Billiard::BilliardBallSpawnBehaviour::GetTableWorldBounds() const {
	const auto tableRect = _tableRectRef.Get();
	if (!tableRect) {
		return {};
	}
	const auto tableNode = tableRect->GetNode();
	if (!tableNode) {
		return {};
	}
	return GetRectangleNodeWorldAabb(*tableNode, *tableRect);
}

std::array<int, 15> Billiard::BilliardBallSpawnBehaviour::BuildRackBallNumbers() const {
	std::array<int, 15> rackNumbers{};

	constexpr int kEightBallSlot = 4;
	constexpr int kBackRowLeftCornerSlot = 10;
	constexpr int kBackRowRightCornerSlot = 14;

	rackNumbers[static_cast<size_t>(kEightBallSlot)] = 8;

	std::vector<int> solids = {1, 2, 3, 4, 5, 6, 7};
	std::vector<int> stripes = {9, 10, 11, 12, 13, 14, 15};

	std::random_device randomDevice;
	std::mt19937 rng(randomDevice());

	std::shuffle(solids.begin(), solids.end(), rng);
	std::shuffle(stripes.begin(), stripes.end(), rng);

	const int cornerSolid = solids.front();
	const int cornerStripe = stripes.front();
	solids.erase(solids.begin());
	stripes.erase(stripes.begin());

	if (std::bernoulli_distribution(0.5)(rng)) {
		rackNumbers[static_cast<size_t>(kBackRowLeftCornerSlot)] = cornerSolid;
		rackNumbers[static_cast<size_t>(kBackRowRightCornerSlot)] = cornerStripe;
	}
	else {
		rackNumbers[static_cast<size_t>(kBackRowLeftCornerSlot)] = cornerStripe;
		rackNumbers[static_cast<size_t>(kBackRowRightCornerSlot)] = cornerSolid;
	}

	std::vector<int> remaining;
	remaining.reserve(solids.size() + stripes.size());
	remaining.insert(remaining.end(), solids.begin(), solids.end());
	remaining.insert(remaining.end(), stripes.begin(), stripes.end());
	std::shuffle(remaining.begin(), remaining.end(), rng);

	size_t remainingIndex = 0;
	for (int slot = 0; slot < 15; ++slot) {
		if (slot == kEightBallSlot || slot == kBackRowLeftCornerSlot || slot == kBackRowRightCornerSlot) {
			continue;
		}
		rackNumbers[static_cast<size_t>(slot)] = remaining[remainingIndex++];
	}

	return rackNumbers;
}
