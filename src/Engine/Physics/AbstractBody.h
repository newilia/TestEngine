#pragma once

#include "Engine/SceneNode.h"
#include "Engine/ComponentHolder.h"
#include "PhysicalComponent.h"

class AbstractBody : public SceneNode {
public:
	AbstractBody();
	~AbstractBody() override;
	virtual sf::FloatRect getBbox() const = 0 ;
	virtual size_t getPointCount() const = 0;
	virtual sf::Vector2f getPointGlobal(std::size_t index) const = 0;
	virtual sf::Vector2f getPosGlobal() const = 0;
	virtual void setPosGlobal(sf::Vector2f pos) = 0;
	void init() override;

	shared_ptr<PhysicalComponent> getPhysicalComponent() const { return findComponent<PhysicalComponent>(); }
};