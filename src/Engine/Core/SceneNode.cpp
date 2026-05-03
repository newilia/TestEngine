#include "SceneNode.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/EditorVisualTheme.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "SceneNode.generated.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <vector>

namespace {

	using Engine::EditorVisualTheme::kHierarchySelectionChildOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionFallbackHalfSize;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlinePadPx;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineThickness;

	std::optional<sf::FloatRect> TryGetHierarchySelectionBounds(const SceneNode& node) {
		const sf::Transform nodeWorld = node.GetWorldTransform();
		if (auto sv = std::dynamic_pointer_cast<ShapeVisualBase>(node.GetVisual())) {
			if (const sf::Shape* shape = sv->GetBaseShape()) {
				sf::Transform full = nodeWorld;
				full *= shape->getTransform();
				return Utils::AxisAlignedBoundsAfterTransform(full, shape->getLocalBounds());
			}
		}
		if (auto tv = std::dynamic_pointer_cast<TextVisual>(node.GetVisual())) {
			if (const sf::Text* text = tv->GetText()) {
				sf::Transform full = nodeWorld;
				full *= text->getTransform();
				return Utils::AxisAlignedBoundsAfterTransform(full, text->getLocalBounds());
			}
		}
		if (auto spv = std::dynamic_pointer_cast<SpriteVisual>(node.GetVisual())) {
			if (const sf::Sprite* sprite = spv->GetSprite()) {
				sf::Transform full = nodeWorld;
				full *= sprite->getTransform();
				return Utils::AxisAlignedBoundsAfterTransform(full, sprite->getLocalBounds());
			}
		}
		return std::nullopt;
	}

	void SortChildrenByDrawOrder(std::vector<shared_ptr<SceneNode>>& nodes) {
		std::stable_sort(nodes.begin(), nodes.end(),
		                 [](const shared_ptr<SceneNode>& a, const shared_ptr<SceneNode>& b) {
			                 int la = 0;
			                 int lb = 0;
			                 if (auto sa = a->FindEntity<RelativeSortingStrategy>()) {
				                 la = sa->GetPriority();
			                 }
			                 if (auto sb = b->FindEntity<RelativeSortingStrategy>()) {
				                 lb = sb->GetPriority();
			                 }
			                 return la < lb;
		                 });
	}

	void DrawAabbOutline(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect& bounds,
	                     const sf::Color& outlineColor) {
		sf::RectangleShape frame;
		frame.setPosition(
		    {bounds.position.x - kHierarchySelectionOutlinePadPx, bounds.position.y - kHierarchySelectionOutlinePadPx});
		frame.setSize({bounds.size.x + 2.f * kHierarchySelectionOutlinePadPx,
		               bounds.size.y + 2.f * kHierarchySelectionOutlinePadPx});
		frame.setFillColor(sf::Color::Transparent);
		frame.setOutlineColor(outlineColor);
		frame.setOutlineThickness(kHierarchySelectionOutlineThickness);
		target.draw(frame, states);
	}

	void DrawNodeHierarchySelectionBounds(const SceneNode& node, sf::RenderTarget& target, sf::RenderStates worldOnly,
	                                      const sf::Color& outlineColor) {
		if (const std::optional<sf::FloatRect> bb = TryGetHierarchySelectionBounds(node)) {
			const sf::FloatRect& b = *bb;
			if (b.size.x > 0.f && b.size.y > 0.f) {
				DrawAabbOutline(target, worldOnly, b, outlineColor);
				return;
			}
		}
		const sf::Vector2f pos = node.GetPosGlobal();
		sf::CircleShape marker(kHierarchySelectionFallbackHalfSize);
		marker.setOrigin({kHierarchySelectionFallbackHalfSize, kHierarchySelectionFallbackHalfSize});
		marker.setPosition(pos);
		marker.setFillColor(sf::Color::Transparent);
		marker.setOutlineColor(outlineColor);
		marker.setOutlineThickness(kHierarchySelectionOutlineThickness);
		target.draw(marker, worldOnly);
	}

	void DrawDescendantHierarchySelectionOutlines(const SceneNode& parent, sf::RenderTarget& target,
	                                              sf::RenderStates worldOnly) {
		std::vector<shared_ptr<SceneNode>> sorted = parent.GetChildren();
		SortChildrenByDrawOrder(sorted);
		for (const auto& child : sorted) {
			if (!child || !child->IsEnabled() || !child->IsVisible()) {
				continue;
			}
			DrawNodeHierarchySelectionBounds(*child, target, worldOnly, kHierarchySelectionChildOutlineColor);
			DrawDescendantHierarchySelectionOutlines(*child, target, worldOnly);
		}
	}

	void DrawHierarchySelectionHighlightIfSelected(const SceneNode& node, sf::RenderTarget& target,
	                                               sf::RenderStates states) {
		const auto selected = Engine::MainContext::GetInstance().GetHierarchySelectedForViewport();
		if (!selected || selected.get() != &node) {
			return;
		}
		sf::RenderStates worldOnly = states;
		worldOnly.transform = sf::Transform{};
		DrawDescendantHierarchySelectionOutlines(node, target, worldOnly);
		DrawNodeHierarchySelectionBounds(node, target, worldOnly, kHierarchySelectionOutlineColor);
	}

} // namespace

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

shared_ptr<Transform> SceneNode::GetLocalTransform() const {
	if (!_localTransform) {
		auto self = std::static_pointer_cast<SceneNode>(const_cast<SceneNode*>(this)->shared_from_this());
		_localTransform = std::make_shared<Transform>();
		_localTransform->AttachTo(self);
	}
	return _localTransform;
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

PhysicsBodyBehaviour* SceneNode::FindPhysicsBody() const {
	for (auto& b : _behaviours) {
		if (auto p = std::dynamic_pointer_cast<PhysicsBodyBehaviour>(b)) {
			return p.get();
		}
	}
	return nullptr;
}

sf::Transform SceneNode::GetWorldTransform() const {
	if (!_worldTransformDirty) {
		return _cachedWorldTransform;
	}
	if (auto parent = GetParent()) {
		_cachedWorldTransform = parent->GetWorldTransform() * GetLocalTransform()->GetTransform();
	}
	else {
		_cachedWorldTransform = GetLocalTransform()->GetTransform();
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

sf::Vector2f SceneNode::GetPosGlobal() const {
	return GetWorldTransform().transformPoint(sf::Vector2f{});
}

void SceneNode::SetPosGlobal(sf::Vector2f pos) {
	if (auto parent = GetParent()) {
		const sf::Vector2f local = parent->GetWorldTransform().getInverse().transformPoint(pos);
		GetLocalTransform()->SetPosition(local);
	}
	else {
		GetLocalTransform()->SetPosition(pos);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isEnabled || !_isVisible) {
		return;
	}

	sf::RenderStates nodeStates = states;
	nodeStates.transform *= GetLocalTransform()->GetTransform();

	if (_visual) {
		_visual->Draw(target, nodeStates);
	}

	std::vector<shared_ptr<SceneNode>> sorted = _children;
	SortChildrenByDrawOrder(sorted);

	for (auto& child : sorted) {
		child->draw(target, nodeStates);
	}

	if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		if (auto debugBehaviour = FindBehaviour<PhysicsDebugBehaviour>()) { // todo fix (некрасиво)
			debugBehaviour->DebugDraw(target, nodeStates);
		}
	}

	DrawHierarchySelectionHighlightIfSelected(*this, target, nodeStates);
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
}

void SceneNode::SetSortingStrategy(const shared_ptr<RelativeSortingStrategy>& sorting) {
	_sortingStrategy = sorting;
	if (_sortingStrategy) {
		_sortingStrategy->AttachTo(shared_from_this());
	}
}

void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child) {
	assert(!HasChild(child));

	_children.push_back(child);
	child->SetParent(shared_from_this());
	child->MarkWorldTransformSubtreeDirty();
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
		node->MarkWorldTransformSubtreeDirty();
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

void SceneNode::IterateBehavioursSafely(const std::function<void(shared_ptr<Behaviour>)>& func) {
	for (size_t i = 0; i < _behaviours.size(); ++i) {
		func(_behaviours[i]);
	}
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

	IterateBehavioursSafely([&](shared_ptr<Behaviour> b) {
		if (!b->_wasInited) {
			b->OnInit();
			b->_wasInited = true;
		}
	});

	for (auto& c : _children) {
		c->NotifyLifecycleInitRecursive();
	}
}

void SceneNode::NotifyLifecycleDeinitRecursive() {
	for (auto& c : _children) {
		c->NotifyLifecycleDeinitRecursive();
	}

	IterateBehavioursSafely([&](shared_ptr<Behaviour> b) {
		if (b->_wasInited) {
			b->OnDeinit();
			b->_wasInited = false;
		}
	});

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
