#include "Scene.h"

#include "Engine/Core/MainContext.h"
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
