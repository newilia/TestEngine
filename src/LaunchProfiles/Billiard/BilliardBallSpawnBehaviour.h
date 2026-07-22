#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/AssetRef.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneObject.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Graphics/Rect.hpp>

#include <array>
#include <string>

namespace Billiard {

	class BilliardBallSpawnBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		/// @method
		void Setup();

	private:
		void SetupShadows(SceneNode& ballNode);

	private:
		void SpawnBall(int ballIndex, sf::Vector2f worldPos);
		[[nodiscard]] std::string FormatTexturePath(int ballIndex) const;
		[[nodiscard]] sf::FloatRect GetTableWorldBounds() const;
		[[nodiscard]] std::array<int, 15> BuildRackBallNumbers() const;

		/// @property
		AssetRef<SceneObject> _ballPrefab;
		/// @property
		RefWrapper<RectangleShapeVisual> _tableRectRef;
		/// @property
		RefWrapper<SceneNode> _ballParent;
		/// @property(tooltip="fmt placeholder for ball id")
		std::string _texturePathMask = "resources/textures/8ball/ball_{}.png";
		/// @property(minValue=0.f)
		float _ballRadius = 12.f;
		/// @property
		std::vector<RefWrapper<SceneNode>> _lightSources;
	};

} // namespace Billiard
