#pragma once

#include "Engine/ComponentHolder.h"
#include "Engine/SceneNode.h"
#include "PhysicalComponent.h"

class AbstractBody : public SceneNode
{
public:
	AbstractBody();
	~AbstractBody() override;
	virtual sf::FloatRect GetBbox() const = 0;
	virtual size_t GetPointCount() const = 0;
	virtual sf::Vector2f GetPointGlobal(std::size_t index) const = 0;
	virtual sf::Vector2f GetPosGlobal() const = 0;
	virtual void SetPosGlobal(sf::Vector2f pos) = 0;
	void Init() override;

	shared_ptr<PhysicalComponent> GetPhysicalComponent() const { return findComponent<PhysicalComponent>(); }
};
