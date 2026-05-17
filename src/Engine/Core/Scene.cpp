#include "Scene.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <optional>
#include <random>
#include <unordered_set>
#include <vector>

namespace {
	using std::shared_ptr;

	shared_ptr<SceneNode> FindTopMostInSubtree(
	    const shared_ptr<SceneNode>& node, const sf::Vector2f& worldPoint, bool tapResponsiveOnly) {
		if (!node) {
			return nullptr;
		}
		std::vector<shared_ptr<SceneNode>> sorted = node->GetChildren();
		Utils::SortSceneNodesByDrawOrder(sorted);
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			if (auto hit = FindTopMostInSubtree(*it, worldPoint, tapResponsiveOnly)) {
				return hit;
			}
		}
		if (auto visual = node->GetVisual();
		    visual && visual->HitTest(worldPoint) && (!tapResponsiveOnly || visual->IsTapHandlingEnabled())) {
			return node;
		}
		return nullptr;
	}

	sf::FloatRect NormalizeRect(const sf::FloatRect& rect) {
		const float minX = std::min(rect.position.x, rect.position.x + rect.size.x);
		const float minY = std::min(rect.position.y, rect.position.y + rect.size.y);
		const float maxX = std::max(rect.position.x, rect.position.x + rect.size.x);
		const float maxY = std::max(rect.position.y, rect.position.y + rect.size.y);
		return {{minX, minY}, {maxX - minX, maxY - minY}};
	}

	bool RectContainsRect(const sf::FloatRect& outer, const sf::FloatRect& inner) {
		return inner.position.x >= outer.position.x && inner.position.y >= outer.position.y &&
		       inner.position.x + inner.size.x <= outer.position.x + outer.size.x &&
		       inner.position.y + inner.size.y <= outer.position.y + outer.size.y;
	}

	bool RectIntersectsRect(const sf::FloatRect& lhs, const sf::FloatRect& rhs) {
		const float lhsRight = lhs.position.x + lhs.size.x;
		const float lhsBottom = lhs.position.y + lhs.size.y;
		const float rhsRight = rhs.position.x + rhs.size.x;
		const float rhsBottom = rhs.position.y + rhs.size.y;
		return lhs.position.x <= rhsRight && lhsRight >= rhs.position.x && lhs.position.y <= rhsBottom &&
		       lhsBottom >= rhs.position.y;
	}

	void FindNodesInRectRec(const shared_ptr<SceneNode>& node, const sf::FloatRect& normalizedRect,
	    Scene::RectSelectionMode mode, std::vector<shared_ptr<SceneNode>>& outNodes) {
		if (!node || !node->IsEnabled() || !node->IsVisible()) {
			return;
		}

		const auto& children = node->GetChildren();
		for (auto it = children.rbegin(); it != children.rend(); ++it) {
			if (!*it) {
				continue;
			}
			FindNodesInRectRec(*it, normalizedRect, mode, outNodes);
		}

		const auto bounds = Utils::TryGetNodeVisualWorldBounds(node);
		if (!bounds) {
			return;
		}

		const bool isMatch = (mode == Scene::RectSelectionMode::kContains)
		                         ? RectContainsRect(normalizedRect, *bounds)
		                         : RectIntersectsRect(normalizedRect, *bounds);
		if (isMatch) {
			outNodes.push_back(node);
		}
	}

} // namespace

Scene::Scene() : _root(SceneNode::Create()) {
	_root->SetName("Root");
}

std::shared_ptr<SceneNode> Scene::GetRoot() const {
	return _root;
}

void Scene::OnEvent(const sf::Event& event) {
	if (auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			auto window = Engine::MainContext::GetInstance().GetMainWindow();
			if (!window) {
				return;
			}
			const sf::Vector2f worldPos = Utils::MapWindowPixelToWorld(*window, pressed->position);
			DispatchTapAt(worldPos);
		}
	}
}

void Scene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_root) {
		_root->draw(target, states);
	}
}

void Scene::Update(const sf::Time& dt) {
	if (_root) {
		_root->Update(dt);
	}
}

void Scene::Init() {
	WireGraphOwningScene();
	RebuildObjectIndex();
	EventHandlerBase::SubscribeForEvents();
	if (_root) {
		_root->NotifyLifecycleInitRecursive();
	}
}

void Scene::WireGraphOwningScene() {
	if (_root) {
		_root->PropagateOwningScene(std::dynamic_pointer_cast<Scene>(shared_from_this()));
	}
}

void Scene::Deinit() {
	EventHandlerBase::UnsubscribeFromEvents();
	if (_root) {
		_root->NotifyLifecycleDeinitRecursive();
		_root->PropagateOwningScene({});
	}
}

void Scene::NotifyPresentRec(const sf::Time& wallFrameDt) {
	if (_root) {
		_root->NotifyPresentRec(wallFrameDt);
	}
}

bool Scene::DispatchTapAt(const sf::Vector2f& worldPoint) {
	if (auto hit = FindTopMostNodeAtPoint(worldPoint)) {
		if (auto v = hit->GetVisual()) {
			v->OnTap(worldPoint);
			return true;
		}
	}
	return false;
}

std::shared_ptr<SceneNode> Scene::FindTopMostNodeAtPoint(const sf::Vector2f& worldPoint, bool tapResponsiveOnly) {
	return FindTopMostInSubtree(_root, worldPoint, tapResponsiveOnly);
}

std::vector<std::shared_ptr<SceneNode>> Scene::FindNodesInRect(
    const sf::FloatRect& worldRect, RectSelectionMode mode) const {
	std::vector<std::shared_ptr<SceneNode>> result;
	const sf::FloatRect normalizedRect = NormalizeRect(worldRect);
	FindNodesInRectRec(_root, normalizedRect, mode, result);
	return result;
}

namespace {

	Engine::SceneObjectId RandomNonZeroSceneObjectId() {
		thread_local std::mt19937 rng{std::random_device{}()};
		std::uniform_int_distribution<std::uint32_t> dist(1u, (std::numeric_limits<std::uint32_t>::max)());
		return dist(rng);
	}

	void EnsureUniqueSceneObjectId(Engine::SceneObjectId& id, std::unordered_set<std::uint32_t>& claimed) {
		if (id != Engine::kInvalidSceneObjectId && !claimed.contains(id)) {
			claimed.insert(id);
			return;
		}
		do {
			id = RandomNonZeroSceneObjectId();
		}
		while (claimed.contains(id));
		claimed.insert(id);
	}

} // namespace

void Scene::MarkSceneObjectIndexDirty() {
	_sceneObjectIndexDirty = true;
}

void Scene::FlushSceneObjectIndexIfDirty() {
	if (!_sceneObjectIndexDirty) {
		return;
	}
	RebuildObjectIndex();
}

void Scene::RebuildObjectIndex() {
	std::unordered_set<std::uint32_t> claimed;
	std::vector<std::pair<std::uint32_t, std::weak_ptr<SceneNode>>> nodeEntries;
	std::vector<std::pair<std::uint32_t, std::weak_ptr<EntityOnNode>>> entityEntries;
	nodeEntries.reserve(256);
	entityEntries.reserve(256);

	const std::function<void(const std::shared_ptr<SceneNode>&)> visit = [&](const std::shared_ptr<SceneNode>& node) {
		if (!node) {
			return;
		}
		Engine::SceneObjectId nid = node->GetSceneObjectId();
		EnsureUniqueSceneObjectId(nid, claimed);
		node->SetSceneObjectId(nid);
		nodeEntries.push_back({nid, node});

		if (const auto visual = node->GetVisual()) {
			Engine::SceneObjectId vid = visual->GetSceneObjectId();
			EnsureUniqueSceneObjectId(vid, claimed);
			visual->SetSceneObjectId(vid);
			entityEntries.push_back({vid, visual});
		}
		if (const auto sorting = node->GetSortingStrategy()) {
			Engine::SceneObjectId sid = sorting->GetSceneObjectId();
			EnsureUniqueSceneObjectId(sid, claimed);
			sorting->SetSceneObjectId(sid);
			entityEntries.push_back({sid, sorting});
		}
		for (const auto& behaviour : node->GetBehaviours()) {
			if (!behaviour) {
				continue;
			}
			Engine::SceneObjectId bid = behaviour->GetSceneObjectId();
			EnsureUniqueSceneObjectId(bid, claimed);
			behaviour->SetSceneObjectId(bid);
			entityEntries.push_back({bid, behaviour});
		}
		for (const auto& child : node->GetChildren()) {
			visit(child);
		}
	};

	_nodesByObjectId.clear();
	_entitiesByObjectId.clear();
	visit(_root);
	for (const auto& [id, w] : nodeEntries) {
		_nodesByObjectId[id] = w;
	}
	for (const auto& [id, w] : entityEntries) {
		_entitiesByObjectId[id] = w;
	}
	_sceneObjectIndexDirty = false;
}

std::shared_ptr<SceneNode> Scene::FindNodeByObjectId(Engine::SceneObjectId id) const {
	if (id == Engine::kInvalidSceneObjectId) {
		return nullptr;
	}
	const auto it = _nodesByObjectId.find(id);
	if (it == _nodesByObjectId.end()) {
		return nullptr;
	}
	return it->second.lock();
}

std::shared_ptr<EntityOnNode> Scene::FindEntityByObjectId(Engine::SceneObjectId id) const {
	if (id == Engine::kInvalidSceneObjectId) {
		return nullptr;
	}
	const auto it = _entitiesByObjectId.find(id);
	if (it == _entitiesByObjectId.end()) {
		return nullptr;
	}
	return it->second.lock();
}
