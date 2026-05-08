#include "Scene.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Utils.h"

#include <algorithm>
#include <optional>
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

Scene::Scene() : _root(std::make_shared<SceneNode>()) {}

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
	EventHandlerBase::SubscribeForEvents();
	if (_root) {
		_root->NotifyLifecycleInitRecursive();
	}
}

void Scene::Deinit() {
	EventHandlerBase::UnsubscribeFromEvents();
	if (_root) {
		_root->NotifyLifecycleDeinitRecursive();
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
