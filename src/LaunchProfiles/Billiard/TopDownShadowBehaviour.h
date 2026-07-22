#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"

#include <memory>

namespace Billiard {

	class TopDownShadowBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnInit() override;
		void OnUpdate(const sf::Time& dt) override;

	public:
		[[nodiscard]] std::weak_ptr<SceneNode> GetLightSource() const;
		void SetLightSource(std::weak_ptr<SceneNode> value);
		[[nodiscard]] const RefWrapper<SceneNode>& GetObject() const;
		void SetObject(RefWrapper<SceneNode> value);
		[[nodiscard]] float GetDistanceFactor() const;
		void SetDistanceFactor(float value);

		void UpdateShadowPosition() const;

	private:
		/// @property
		RefWrapper<SceneNode> _object;
		/// @property(dragSpeed=0.001f, minValue=0.f)
		float _distanceFactor = 0.f;

	private:
		std::weak_ptr<SceneNode> _lightSource;
	};

} // namespace Billiard
