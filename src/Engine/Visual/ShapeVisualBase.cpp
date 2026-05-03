#include "Engine/Visual/ShapeVisualBase.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "ShapeVisualBase.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

ShapeVisualBase::ShapeVisualBase() {}

const sf::Shape* ShapeVisualBase::GetBaseShape() const {
	return const_cast<ShapeVisualBase*>(this)->GetBaseShape();
}

bool ShapeVisualBase::HitTest(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	const sf::Transform nodeWorld = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfShape(worldPoint, GetBaseShape(), nodeWorld);
}

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto shape = GetBaseShape()) {
		target.draw(*shape, states);
	}
}

sf::Color ShapeVisualBase::GetFillColor() {
	if (auto shape = GetBaseShape()) {
		return shape->getFillColor();
	}
	return sf::Color::Transparent;
}

void ShapeVisualBase::SetFillColor(const sf::Color& color) {
	if (auto shape = GetBaseShape()) {
		shape->setFillColor(color);
	}
}

sf::Color ShapeVisualBase::GetOutlineColor() {
	if (auto shape = GetBaseShape()) {
		return shape->getOutlineColor();
	}
	return sf::Color::Transparent;
}

void ShapeVisualBase::SetOutlineColor(const sf::Color& color) {
	if (auto shape = GetBaseShape()) {
		shape->setOutlineColor(color);
	}
}

float ShapeVisualBase::GetOutlineThickness() {
	if (auto shape = GetBaseShape()) {
		return shape->getOutlineThickness();
	}
	return 0.f;
}

void ShapeVisualBase::SetOutlineThickness(float thickness) {
	if (auto shape = GetBaseShape()) {
		shape->setOutlineThickness(thickness);
	}
}
