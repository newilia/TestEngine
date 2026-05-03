#include "Engine/Visual/ShapeVisualBase.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "ShapeVisualBase.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>

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

sf::Vector2f ShapeVisualBase::GetPosition() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getPosition();
	}
	return {};
}

void ShapeVisualBase::SetPosition(const sf::Vector2f& position) {
	if (auto shape = GetBaseShape()) {
		shape->setPosition(position);
	}
}

sf::Angle ShapeVisualBase::GetRotation() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getRotation();
	}
	return sf::degrees(0.f);
}

void ShapeVisualBase::SetRotation(sf::Angle angle) {
	if (auto shape = GetBaseShape()) {
		shape->setRotation(angle);
	}
}

sf::Vector2f ShapeVisualBase::GetScale() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getScale();
	}
	return {1.f, 1.f};
}

void ShapeVisualBase::SetScale(const sf::Vector2f& scale) {
	if (auto shape = GetBaseShape()) {
		shape->setScale(scale);
	}
}

sf::Vector2f ShapeVisualBase::GetOrigin() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getOrigin();
	}
	return {};
}

void ShapeVisualBase::SetOrigin(const sf::Vector2f& origin) {
	if (auto shape = GetBaseShape()) {
		shape->setOrigin(origin);
	}
}

float ShapeVisualBase::GetMiterLimit() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getMiterLimit();
	}
	return 10.f;
}

void ShapeVisualBase::SetMiterLimit(float miterLimit) {
	if (auto shape = GetBaseShape()) {
		shape->setMiterLimit(miterLimit);
	}
}
