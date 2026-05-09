#include "Engine/Visual/ShapeVisualBase.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/ShapeLightingDraw.h"
#include "ShapeVisualBase.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>

sf::Shape* ShapeVisualBase::GetBaseShapeToChange() {
	return const_cast<sf::Shape*>(GetBaseShape());
}

bool ShapeVisualBase::HitTest(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	const sf::Transform nodeWorld = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfShape(worldPoint, GetBaseShape(), nodeWorld);
}

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto shape = GetBaseShape()) {
		if (Engine::TryDrawShapeWithLighting(*this, target, states)) {
			return;
		}
		target.draw(*shape, states);
	}
}

const sf::Transform* ShapeVisualBase::GetTransform() const {
	return &GetBaseShape()->getTransform();
}

sf::Color ShapeVisualBase::GetFillColor() const {
	if (auto shape = GetBaseShape()) {
		return shape->getFillColor();
	}
	return sf::Color::Transparent;
}

void ShapeVisualBase::SetFillColor(const sf::Color& color) {
	if (auto shape = GetBaseShapeToChange()) {
		shape->setFillColor(color);
	}
}

sf::Color ShapeVisualBase::GetOutlineColor() const {
	if (auto shape = GetBaseShape()) {
		return shape->getOutlineColor();
	}
	return sf::Color::Transparent;
}

void ShapeVisualBase::SetOutlineColor(const sf::Color& color) {
	if (auto shape = GetBaseShapeToChange()) {
		shape->setOutlineColor(color);
	}
}

float ShapeVisualBase::GetOutlineThickness() const {
	if (auto shape = GetBaseShape()) {
		return shape->getOutlineThickness();
	}
	return 0.f;
}

void ShapeVisualBase::SetOutlineThickness(float thickness) {
	if (auto shape = GetBaseShapeToChange()) {
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
	if (auto shape = GetBaseShapeToChange()) {
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
	if (auto shape = GetBaseShapeToChange()) {
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
	if (auto shape = GetBaseShapeToChange()) {
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
	if (auto shape = GetBaseShapeToChange()) {
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
	if (auto shape = GetBaseShapeToChange()) {
		shape->setMiterLimit(miterLimit);
	}
}

sf::FloatRect ShapeVisualBase::GetLocalBounds() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getLocalBounds();
	}
	return {};
}

sf::FloatRect ShapeVisualBase::GetGlobalBounds() const {
	if (const auto* shape = GetBaseShape()) {
		return shape->getGlobalBounds();
	}
	return {};
}
