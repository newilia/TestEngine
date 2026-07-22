#include "TopDownShadowBehaviour.h"

#include "Engine/Core/SceneNodeUtils.h"
#include "TopDownShadowBehaviour.generated.hpp"

namespace Billiard {

	std::weak_ptr<SceneNode> TopDownShadowBehaviour::GetLightSource() const {
		return _lightSource;
	}

	void TopDownShadowBehaviour::SetLightSource(std::weak_ptr<SceneNode> value) {
		_lightSource = std::move(value);
	}

	const RefWrapper<SceneNode>& TopDownShadowBehaviour::GetObject() const {
		return _object;
	}

	void TopDownShadowBehaviour::SetObject(RefWrapper<SceneNode> value) {
		_object = std::move(value);
	}

	float TopDownShadowBehaviour::GetDistanceFactor() const {
		return _distanceFactor;
	}

	void TopDownShadowBehaviour::SetDistanceFactor(float value) {
		_distanceFactor = value;
	}

	void TopDownShadowBehaviour::OnInit() {
		UpdateShadowPosition();
	}

	void TopDownShadowBehaviour::OnUpdate(const sf::Time& /*dt*/) {
		UpdateShadowPosition();
	}

	void TopDownShadowBehaviour::UpdateShadowPosition() const {
		const auto shadowNode = GetNode();
		const auto objectNode = _object.Get();
		const auto lightNode = _lightSource.lock();
		if (!shadowNode || !objectNode || !lightNode) {
			return;
		}

		const sf::Vector2f objectWorld = Utils::GetWorldPos(objectNode);
		const sf::Vector2f lightWorld = Utils::GetWorldPos(lightNode);
		const sf::Vector2f worldOffset = (objectWorld - lightWorld) * _distanceFactor;

		if (const auto parent = shadowNode->GetParent()) {
			const sf::Transform parentWorldInv = parent->GetWorldTransform().getInverse();
			const sf::Vector2f localOffset =
			    parentWorldInv.transformPoint(worldOffset) - parentWorldInv.transformPoint(sf::Vector2f{});
			shadowNode->SetLocalPosition(localOffset);
		}
		else {
			shadowNode->SetLocalPosition(worldOffset);
		}
	}
} // namespace Billiard
