#pragma once

#include "Engine/SceneNode.h"
#include "PhysicalBehaviour.h"

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

	shared_ptr<PhysicalBehaviour> GetPhysicalComponent() const {
		auto* self = const_cast<AbstractBody*>(this);
		if (!self->FindEntity<PhysicalBehaviour>()) {
			self->RequireEntity<PhysicalBehaviour>();
		}
		return FindEntity<PhysicalBehaviour>();
	}
};
