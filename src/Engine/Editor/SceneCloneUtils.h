#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/IPropertiesProvider.h"

#include <memory>

class Behaviour;
class RelativeSortingStrategy;
class SceneNode;
class Transform;
class Visual;

namespace Engine {

	enum class EntitySlot
	{
		Transform,
		Visual,
		SortingStrategy,
		Behaviour,
	};

	std::shared_ptr<EntityOnNode> CloneEntity(const std::shared_ptr<EntityOnNode>& source);
	std::shared_ptr<SceneNode> CloneSceneNode(const std::shared_ptr<SceneNode>& source);

	/// Applies source provider values onto target provider for reflected properties.
	bool CopyReflectedProperties(IPropertiesProvider& source, IPropertiesProvider& target);

} // namespace Engine
