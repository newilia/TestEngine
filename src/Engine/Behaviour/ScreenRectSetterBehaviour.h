#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

class ScreenRectSetterBehaviour : public Behaviour
{
	META_CLASS()

public:
	void OnInit() override;
};
