#include "SceneNode.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "SceneNode.generated.hpp"

#include <cassert>
#include <memory>
#include <vector>

void SceneNode::SetName(const std::string& name) {
	_name = name;
}

const std::string& SceneNode::GetName() const {
	return _name;
}

Engine::SceneObjectId SceneNode::GetSceneObjectId() const {
	return _sceneObjectId;
}

void SceneNode::SetSceneObjectId(Engine::SceneObjectId id) {
	_sceneObjectId = id;
}

shared_ptr<SceneNode> SceneNode::GetParent() const {
	return _parent.lock();
}

const std::vector<shared_ptr<SceneNode>>& SceneNode::GetChildren() const {
	return _children;
}

shared_ptr<Visual> SceneNode::GetVisual() const {
	return _visual;
}

std::shared_ptr<SceneNode> SceneNode::Create() {
	return std::make_shared<SceneNode>();
}

Transform& SceneNode::GetLocalTransform() {
	return _localTransform;
}

const Transform& SceneNode::GetLocalTransform() const {
	return _localTransform;
}

sf::Vector2f SceneNode::GetLocalPosition() const {
	return _localTransform.GetPosition();
}

void SceneNode::SetLocalPosition(sf::Vector2f v) {
	_localTransform.SetPosition(v);
	MarkWorldTransformSubtreeDirty();
}

sf::Vector2f SceneNode::GetLocalScale() const {
	return _localTransform.GetScale();
}

void SceneNode::SetLocalScale(sf::Vector2f v) {
	_localTransform.SetScale(v);
	MarkWorldTransformSubtreeDirty();
}

sf::Angle SceneNode::GetLocalRotation() const {
	return _localTransform.GetRotation();
}

void SceneNode::SetLocalRotation(sf::Angle angle) {
	_localTransform.SetRotation(angle);
	MarkWorldTransformSubtreeDirty();
}

sf::Vector2f SceneNode::GetLocalOrigin() const {
	return _localTransform.GetOrigin();
}

void SceneNode::SetLocalOrigin(sf::Vector2f v) {
	_localTransform.SetOrigin(v);
	MarkWorldTransformSubtreeDirty();
}

void SceneNode::CopyLocalTransformFrom(const Transform& source) {
	_localTransform.SetPosition(source.GetPosition());
	_localTransform.SetScale(source.GetScale());
	_localTransform.SetRotation(source.GetRotation());
	_localTransform.SetOrigin(source.GetOrigin());
	MarkWorldTransformSubtreeDirty();
}

shared_ptr<RelativeSortingStrategy> SceneNode::GetSortingStrategy() const {
	return _sortingStrategy;
}

const std::vector<shared_ptr<Behaviour>>& SceneNode::GetBehaviours() const {
	return _behaviours;
}

bool SceneNode::IsEnabled() const {
	return _isEnabled;
}

bool SceneNode::IsVisible() const {
	return _isVisible;
}

void SceneNode::SetParent(const shared_ptr<SceneNode>& parent) {
	_parent = parent;
}

const sf::Transform& SceneNode::GetWorldTransform() const {
	if (!_worldTransformDirty) {
		return _cachedWorldTransform;
	}
	if (auto parent = GetParent()) {
		_cachedWorldTransform = parent->GetWorldTransform() * GetLocalTransform().GetTransform();
	}
	else {
		_cachedWorldTransform = GetLocalTransform().GetTransform();
	}
	_worldTransformDirty = false;
	return _cachedWorldTransform;
}

void SceneNode::MarkWorldTransformSubtreeDirty() const {
	_worldTransformDirty = true;
	for (const auto& child : _children) {
		child->MarkWorldTransformSubtreeDirty();
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	sf::RenderStates nodeStates = states;
	nodeStates.transform *= GetLocalTransform().GetTransform();

	if (_visual) {
		_visual->Draw(target, nodeStates);
	}

	std::vector<shared_ptr<SceneNode>> sorted = _children;
	Utils::SortSceneNodesByDrawOrder(sorted);

	for (auto& child : sorted) {
		child->draw(target, nodeStates);
	}
}

void SceneNode::Update(const sf::Time& dt) {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	for (auto& b : _behaviours) {
		b->OnUpdate(dt);
	}
	for (auto& child : _children) {
		child->Update(dt);
	}
}

void SceneNode::NotifyPresentRec(const sf::Time& realFrameDt) {
	if (!_isEnabled || !_isVisible) {
		return;
	}
	for (auto& b : _behaviours) {
		b->OnPresent(realFrameDt);
	}
	for (auto& child : _children) {
		child->NotifyPresentRec(realFrameDt);
	}
}

void SceneNode::RemoveFromParent() {
	if (auto parent = GetParent()) {
		parent->RemoveChild(this);
	}
}

void SceneNode::SetVisual(shared_ptr<Visual>&& visual) {
	_visual = std::move(visual);
	if (_visual) {
		_visual->AttachTo(shared_from_this());
	}
	NotifyActiveSceneObjectIndexDirty();
}

void SceneNode::SetSortingStrategy(const shared_ptr<RelativeSortingStrategy>& sorting) {
	_sortingStrategy = sorting;
	if (_sortingStrategy) {
		_sortingStrategy->AttachTo(shared_from_this());
	}
	NotifyActiveSceneObjectIndexDirty();
}

void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child) {
	AddChildAt(child, _children.size());
}

void SceneNode::AddChildAt(const std::shared_ptr<SceneNode>& child, std::size_t index) {
	assert(!HasChild(child));

	if (index > _children.size()) {
		index = _children.size();
	}
	_children.insert(_children.begin() + static_cast<std::ptrdiff_t>(index), child);
	child->SetParent(shared_from_this());
	child->PropagateOwningScene(_owningScene);
	child->MarkWorldTransformSubtreeDirty();
	if (IsInActiveScene()) {
		child->NotifyLifecycleInitRecursive();
	}
	NotifyActiveSceneObjectIndexDirty();
}

void SceneNode::RemoveChild(SceneNode* child) {
	auto it = std::ranges::find_if(_children.begin(), _children.end(), [child](const auto& ptr) {
		return ptr.get() == child;
	});
	if (it == _children.end()) {
		return;
	}
	shared_ptr<SceneNode> node = *it;
	if (node->IsInActiveScene()) {
		node->NotifyLifecycleDeinitRecursive();
	}
	node->PropagateOwningScene({});
	node->SetParent(nullptr);
	node->MarkWorldTransformSubtreeDirty();
	_children.erase(it);
	NotifyActiveSceneObjectIndexDirty();
}

shared_ptr<SceneNode> SceneNode::FindChild(const std::string& id, bool recursively) {
	for (auto& child : _children) {
		if (child->GetName() == id) {
			return child;
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			if (auto grandChild = child->FindChild(id, true)) {
				return grandChild;
			}
		}
	}
	return nullptr;
}

bool SceneNode::HasChild(const std::shared_ptr<SceneNode>& child) {
	auto it = std::find(_children.begin(), _children.end(), child);
	return it != _children.end();
}

std::vector<shared_ptr<SceneNode>> SceneNode::FindChildren(const std::string& id, bool recursively) {
	std::vector<shared_ptr<SceneNode>> result;
	for (auto& child : _children) {
		if (child->GetName() == id) {
			result.emplace_back(child);
		}
	}
	if (recursively) {
		for (auto& child : _children) {
			auto grandChildren = child->FindChildren(id, true);
			result.insert(result.end(), grandChildren.begin(), grandChildren.end());
		}
	}
	return result;
}

void SceneNode::AddBehaviour(shared_ptr<Behaviour> behaviour) {
	behaviour->AttachTo(shared_from_this());
	_behaviours.push_back(std::move(behaviour));
	_behaviours.back()->OnAttached();
	NotifyActiveSceneObjectIndexDirty();
}

void SceneNode::RemoveBehaviour(Behaviour* behaviour) {
	auto it = std::ranges::find_if(_behaviours.begin(), _behaviours.end(), [behaviour](const auto& ptr) {
		return ptr.get() == behaviour;
	});
	if (it == _behaviours.end()) {
		return;
	}
	DetachBehaviourForRemove(*it);
	_behaviours.erase(it);
	NotifyActiveSceneObjectIndexDirty();
}

void SceneNode::SetEnabled(bool isEnabled) {
	if (_isEnabled == isEnabled) {
		return;
	}
	_isEnabled = isEnabled;
	NotifyEnabledRecursive(isEnabled);
}

void SceneNode::SetVisible(bool isVisible) {
	if (_isVisible == isVisible) {
		return;
	}
	_isVisible = isVisible;
	NotifyVisibleRecursive(isVisible);
}

void SceneNode::NotifyEnabledRecursive(bool isEnabled) {
	for (auto& b : _behaviours) {
		b->OnEnabled(isEnabled);
	}
	for (auto& child : _children) {
		child->NotifyEnabledRecursive(isEnabled);
	}
}

void SceneNode::NotifyVisibleRecursive(bool isVisible) {
	for (auto& b : _behaviours) {
		b->OnVisible(isVisible);
	}
	for (auto& child : _children) {
		child->NotifyVisibleRecursive(isVisible);
	}
}

void SceneNode::PropagateOwningScene(const weak_ptr<Scene>& scene) {
	_owningScene = scene;
	for (const auto& c : _children) {
		c->PropagateOwningScene(scene);
	}
}

void SceneNode::IterateBehavioursSafely(const std::function<void(shared_ptr<Behaviour>)>& func) {
	/* TODO validate */
	for (size_t i = 0; i < _behaviours.size(); ++i) {
		func(_behaviours[i]);
	}
}

bool SceneNode::IsInActiveScene() const {
	if (const auto owned = _owningScene.lock()) {
		return owned == Engine::MainContext::GetInstance().GetScene();
	}
	return false;
}

void SceneNode::NotifyActiveSceneObjectIndexDirty() const {
	if (const auto scene = _owningScene.lock()) {
		scene->MarkSceneObjectIndexDirty();
	}
}

void SceneNode::NotifyLifecycleInitRecursive() {
	if (_wasNodeLifecycleInited) {
		return;
	}
	_wasNodeLifecycleInited = true;

	for (auto& c : _children) {
		c->NotifyLifecycleInitRecursive();
	}

	IterateBehavioursSafely([&](shared_ptr<Behaviour> b) {
		if (!b->_wasInited) {
			b->OnInit();
			b->_wasInited = true;
		}
	});
}

void SceneNode::NotifyLifecycleDeinitRecursive() {
	if (!_wasNodeLifecycleInited) {
		return;
	}
	_wasNodeLifecycleInited = false;

	IterateBehavioursSafely([&](shared_ptr<Behaviour> b) {
		if (b->_wasInited) {
			b->OnDeinit();
			b->_wasInited = false;
		}
	});

	for (auto& c : _children) {
		c->NotifyLifecycleDeinitRecursive();
	}
}

void SceneNode::DetachBehaviourForRemove(const shared_ptr<Behaviour>& b) {
	if (b->_wasInited) {
		b->OnDeinit();
		b->_wasInited = false;
	}
	b->OnDetached();
}

void SceneNode::FindNodesAtPoint(
    const sf::Vector2f& worldPoint, std::vector<shared_ptr<SceneNode>>& result, bool tapResponsiveOnly) {
	std::vector<shared_ptr<SceneNode>> sorted = _children;
	Utils::SortSceneNodesByDrawOrder(sorted);
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
		(*it)->FindNodesAtPoint(worldPoint, result, tapResponsiveOnly);
	}
	if (_visual && _visual->HitTest(worldPoint) && (!tapResponsiveOnly || _visual->IsTapHandlingEnabled())) {
		result.push_back(shared_from_this());
	}
}
