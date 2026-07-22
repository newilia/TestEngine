#include "BilliardBallSpawnBehaviour.h"

#include "BilliardBallSpawnBehaviour.generated.hpp"
#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeClone.h"
#include "Engine/Core/SceneNodeUtils.h"

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
}

std::string Billiard::BilliardBallSpawnBehaviour::FormatTexturePath(int ballIndex) const {
	const std::string idPart = ballIndex == 0 ? "cue" : fmt::format("{:02d}", ballIndex);
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
	rackNumbers[static_cast<size_t>(kBackRowLeftCornerSlot)] = 1;
	rackNumbers[static_cast<size_t>(kBackRowRightCornerSlot)] = 9;

	std::vector<int> remainingSolids = {2, 3, 4, 5, 6, 7};
	std::vector<int> remainingStripes = {10, 11, 12, 13, 14, 15};
	std::vector<int> remaining;
	remaining.reserve(remainingSolids.size() + remainingStripes.size());
	remaining.insert(remaining.end(), remainingSolids.begin(), remainingSolids.end());
	remaining.insert(remaining.end(), remainingStripes.begin(), remainingStripes.end());

	std::mt19937 rng(0);
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
