#include "SceneNode.h"

#include "Engine/App/MainContext.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Core/Transform.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/TextVisual.h"
#include "SceneNode.generated.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>

namespace {

	constexpr float kSelectionOutlinePadPx = 3.f;
	constexpr float kSelectionOutlineThickness = 2.f;
	const sf::Color kSelectionOutlineColor(120u, 190u, 255u, 220u);
	constexpr float kSelectionFallbackHalfSize = 6.f;

	void DrawAabbOutline(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect& bounds) {
		sf::RectangleShape frame;
		frame.setPosition({bounds.position.x - kSelectionOutlinePadPx, bounds.position.y - kSelectionOutlinePadPx});
		frame.setSize({bounds.size.x + 2.f * kSelectionOutlinePadPx, bounds.size.y + 2.f * kSelectionOutlinePadPx});
		frame.setFillColor(sf::Color::Transparent);
		frame.setOutlineColor(kSelectionOutlineColor);
		frame.setOutlineThickness(kSelectionOutlineThickness);
		target.draw(frame, states);
	}

	std::optional<sf::FloatRect> TryGetHierarchySelectionBounds(const SceneNode& node) {
		if (auto sv = std::dynamic_pointer_cast<ShapeVisualBase>(node.GetVisual())) {
			if (sf::Shape* shape = sv->GetShape()) {
				return shape->getGlobalBounds();
			}
		}
		if (auto tv = std::dynamic_pointer_cast<TextVisual>(node.GetVisual())) {
			if (const sf::Text* text = tv->GetText()) {
				return text->getGlobalBounds();
			}
		}
		if (auto* c = node.FindShapeCollider()) {
			return c->GetBbox();
		}
		return std::nullopt;
	}

	void DrawHierarchySelectionHighlightIfSelected(const SceneNode& node, sf::RenderTarget& target,
	                                               sf::RenderStates states) {
		const auto selected = Engine::MainContext::GetInstance().GetHierarchySelectedForViewport();
		if (!selected || selected.get() != &node) {
			return;
		}
		if (const std::optional<sf::FloatRect> bb = TryGetHierarchySelectionBounds(node)) {
			const sf::FloatRect& b = *bb;
			if (b.size.x > 0.f && b.size.y > 0.f) {
				DrawAabbOutline(target, states, b);
				return;
			}
		}
		const sf::Vector2f pos = node.GetPosGlobal();
		sf::CircleShape marker(kSelectionFallbackHalfSize);
		marker.setOrigin({kSelectionFallbackHalfSize, kSelectionFallbackHalfSize});
		marker.setPosition(pos);
		marker.setFillColor(sf::Color::Transparent);
		marker.setOutlineColor(kSelectionOutlineColor);
		marker.setOutlineThickness(kSelectionOutlineThickness);
		target.draw(marker, states);
	}

	void SortChildrenByDrawOrder(std::vector<shared_ptr<SceneNode>>& nodes) {
		std::stable_sort(nodes.begin(), nodes.end(),
		                 [](const shared_ptr<SceneNode>& a, const shared_ptr<SceneNode>& b) {
			                 int la = 0;
			                 int lb = 0;
			                 if (auto sa = a->FindEntity<SortingStrategy>()) {
				                 la = sa->GetSortLayer();
			                 }
			                 if (auto sb = b->FindEntity<SortingStrategy>()) {
				                 lb = sb->GetSortLayer();
			                 }
			                 return la < lb;
		                 });
	}

} // namespace

void SceneNode::Update(const sf::Time& /*dt*/) {}

void SceneNode::OnInit() {}

void SceneNode::OnDeinit() {}

void SceneNode::SetName(const std::string& name) {
	_name = name;
}

const std::string& SceneNode::GetName() const {
	return _name;
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

shared_ptr<Transform> SceneNode::GetTransform() const {
	if (!_transform) {
		auto self = std::static_pointer_cast<SceneNode>(const_cast<SceneNode*>(this)->shared_from_this());
		_transform = std::make_shared<Transform>();
		_transform->AttachTo(self);
	}
	return _transform;
}

shared_ptr<SortingStrategy> SceneNode::GetSortingStrategy() const {
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

ShapeColliderBehaviourBase* SceneNode::FindShapeCollider() const {
	for (auto& b : _behaviours) {
		if (auto s = std::dynamic_pointer_cast<ShapeColliderBehaviourBase>(b)) {
			return s.get();
		}
	}
	return nullptr;
}

sf::Vector2f SceneNode::GetPosGlobal() const {
	if (auto* c = FindShapeCollider()) {
		return c->GetPosGlobal();
	}
	return GetTransform()->getPosition();
}

void SceneNode::SetPosGlobal(sf::Vector2f pos) {
	if (auto* c = FindShapeCollider()) {
		c->SetPosGlobal(pos);
	}
	else {
		GetTransform()->setPosition(pos);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	if (_visual) {
		_visual->Draw(target, states);
	}

	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);

	for (auto& child : sorted) {
		child->draw(target, states);
	}

	if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		if (auto debugBehaviour = FindBehaviour<PhysicsDebugBehaviour>()) { // todo fix (некрасиво)
			debugBehaviour->DebugDraw(target, states);
		}
	}

	DrawHierarchySelectionHighlightIfSelected(*this, target, states);
}

void SceneNode::UpdateRec(const sf::Time& dt) {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	Update(dt);
	for (auto& b : _behaviours) {
		b->OnUpdate(dt);
	}
	for (auto& child : _children) {
		child->UpdateRec(dt);
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
}

void SceneNode::SetSortingStrategy(const shared_ptr<SortingStrategy>& sorting) {
	_sortingStrategy = sorting;
	if (_sortingStrategy) {
		_sortingStrategy->AttachTo(shared_from_this());
	}
}

void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child) {
	assert(!HasChild(child));

	_children.push_back(child);
	child->SetParent(shared_from_this());
}

void SceneNode::RemoveChild(SceneNode* child) {
	auto it = std::ranges::find_if(_children.begin(), _children.end(), [child](const auto& ptr) {
		return ptr.get() == child;
	});
	if (it != _children.end()) {
		shared_ptr<SceneNode> node = *it;
		if (node->IsInActiveScene()) {
			node->NotifyLifecycleDeinitRecursive();
		}
		node->SetParent(nullptr);
		_children.erase(it);
	}
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
}

void SceneNode::SetEnabled(bool isEnabled) {
	if (_isEnabled == isEnabled) {
		return;
	}
	_isEnabled = isEnabled;
	for (auto& b : _behaviours) {
		b->OnEnabled(isEnabled);
	}
}

void SceneNode::SetVisible(bool isVisible) {
	if (_isVisible == isVisible) {
		return;
	}
	_isVisible = isVisible;
	for (auto& b : _behaviours) {
		b->OnVisible(isVisible);
	}
}

shared_ptr<SceneNode> SceneNode::GetSubtreeRoot() const {
	auto cur = std::const_pointer_cast<SceneNode>(shared_from_this());
	while (auto p = cur->GetParent()) {
		cur = std::move(p);
	}
	return cur;
}

bool SceneNode::IsInActiveScene() const {
	auto active = Engine::MainContext::GetInstance().GetScene();
	if (!active) {
		return false;
	}
	auto root = GetSubtreeRoot();
	return root && root.get() == active.get();
}

void SceneNode::NotifyLifecycleInitRecursive() {
	if (!_wasNodeLifecycleInited) {
		OnInit();
		_wasNodeLifecycleInited = true;
	}
	for (auto& b : _behaviours) {
		if (!b->_wasInited) {
			b->OnInit();
			b->_wasInited = true;
		}
	}
	for (auto& c : _children) {
		c->NotifyLifecycleInitRecursive();
	}
}

void SceneNode::NotifyLifecycleDeinitRecursive() {
	for (auto& c : _children) {
		c->NotifyLifecycleDeinitRecursive();
	}
	for (auto& b : _behaviours) {
		if (b->_wasInited) {
			b->OnDeinit();
			b->_wasInited = false;
		}
	}
	if (_wasNodeLifecycleInited) {
		OnDeinit();
		_wasNodeLifecycleInited = false;
	}
}

void SceneNode::DetachBehaviourForRemove(const shared_ptr<Behaviour>& b) {
	if (b->_wasInited) {
		b->OnDeinit();
		b->_wasInited = false;
	}
	b->OnDetached();
}

shared_ptr<SceneNode> SceneNode::FindTopMostNodeAtPoint(const sf::Vector2f& worldPoint, bool tapResponsiveOnly) {
	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
		if (auto hit = (*it)->FindTopMostNodeAtPoint(worldPoint, tapResponsiveOnly)) {
			return hit;
		}
	}
	if (_visual && _visual->HitTest(worldPoint) && (!tapResponsiveOnly || _visual->IsTapHandlingEnabled())) {
		return shared_from_this();
	}
	return nullptr;
}

void SceneNode::FindNodesAtPoint(const sf::Vector2f& worldPoint, std::vector<shared_ptr<SceneNode>>& result,
                                 bool tapResponsiveOnly) {
	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
		(*it)->FindNodesAtPoint(worldPoint, result, tapResponsiveOnly);
	}
	if (_visual && _visual->HitTest(worldPoint) && (!tapResponsiveOnly || _visual->IsTapHandlingEnabled())) {
		result.push_back(shared_from_this());
	}
}

bool SceneNode::DispatchTapAt(const sf::Vector2f& worldPoint) {
	if (auto hit = FindTopMostNodeAtPoint(worldPoint)) {
		if (auto v = hit->GetVisual()) {
			v->OnTap(worldPoint);
			return true;
		}
	}
	return false;
}
