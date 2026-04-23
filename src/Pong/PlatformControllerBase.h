#pragma once
#include "Engine/Updateable.h"

class PongPlatform;

namespace sf {
class Event;
}

class PlatformControllerBase : public Updateable
{
public:
	explicit PlatformControllerBase(PongPlatform* platform) : _platform(platform) {}

	virtual void init() {}

protected:
	PongPlatform* _platform = nullptr;
};
