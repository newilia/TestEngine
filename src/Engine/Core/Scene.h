#pragma once

#include "Engine/Core/EntityId.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/EventHandlerBase.h"
#include "SceneNode.h"
#include "Updatable.h"

#include <SFML/Graphics.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

/// Owns the scene graph root; hit picking and tap dispatch are scene-level concerns.
class Scene final : public sf::Drawable, public Updatable, public Engine::EventHandlerBase
{
public:
	enum class RectSelectionMode
	{
		kIntersects,
		kContains
	};

	Scene();
	~Scene() override = default;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void Update(const sf::Time& dt) override;
	void OnEvent(const sf::Event& event) override;

public:
	void Init();
	void Deinit();
	std::shared_ptr<SceneNode> GetRoot() const;
	void NotifyPresentRec(const sf::Time& wallFrameDt); // TODO is it necessary?
	bool DispatchTapAt(const sf::Vector2f& worldPoint);
	[[nodiscard]] std::shared_ptr<SceneNode> FindTopMostNodeAtPoint(
	    const sf::Vector2f& worldPoint, bool tapResponsiveOnly = false);
	[[nodiscard]] std::vector<std::shared_ptr<SceneNode>> FindNodesInRect(
	    const sf::FloatRect& worldRect, RectSelectionMode mode) const;

	void RebuildEntityIndex();
	void MarkEntityIndexDirty();
	void FlushEntityIndexIfDirty();
	std::shared_ptr<SceneNode> FindNodeByEntityId(Engine::EntityId id) const;
	std::shared_ptr<EntityOnNode> FindEntityByEntityId(Engine::EntityId id) const;

private:
	void WireGraphOwningScene();

	std::shared_ptr<SceneNode> _root;

	std::unordered_map<std::uint32_t, std::weak_ptr<SceneNode>> _nodesByEntityId;
	std::unordered_map<std::uint32_t, std::weak_ptr<EntityOnNode>> _entitiesByEntityId;
	bool _entityIndexDirty = false;
};
