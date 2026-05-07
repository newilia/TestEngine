#include "Scene.h"

#include "Engine/Core/Utils.h"

#include <vector>

namespace {

	using std::shared_ptr;

	shared_ptr<SceneNode> FindTopMostInSubtree(const shared_ptr<SceneNode>& node, const sf::Vector2f& worldPoint,
	                                           bool tapResponsiveOnly) {
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

} // namespace

Scene::Scene() : _root(std::make_shared<SceneNode>()) {}

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

void Scene::NotifyPresentRec(const sf::Time& wallFrameDt) {
	if (_root) {
		_root->NotifyPresentRec(wallFrameDt);
	}
}

void Scene::NotifyLifecycleInitRecursive() {
	if (_root) {
		_root->NotifyLifecycleInitRecursive();
	}
}

void Scene::NotifyLifecycleDeinitRecursive() {
	if (_root) {
		_root->NotifyLifecycleDeinitRecursive();
	}
}

void Scene::AddChild(const std::shared_ptr<SceneNode>& child) {
	if (_root) {
		_root->AddChild(child);
	}
}

void Scene::AddChildAt(const std::shared_ptr<SceneNode>& child, std::size_t index) {
	if (_root) {
		_root->AddChildAt(child, index);
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
