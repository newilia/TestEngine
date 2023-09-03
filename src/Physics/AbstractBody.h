#pragma once

#include "SceneNode.h"
#include "ComponentHolder.h"
#include "PhysicalComponent.h"

class AbstractBody : public SceneNode {
public:
	AbstractBody();
	virtual sf::FloatRect getBbox() const = 0 ;
	virtual size_t getPointCount() const = 0;
	virtual sf::Vector2f getPointGlobal(std::size_t index) const = 0;
	virtual sf::Vector2f getGlobalCenter() const = 0;
	void init() override;

	shared_ptr<PhysicalComponent> getPhysicalComponent() const { return findComponent<PhysicalComponent>(); }
};