#pragma once
#include "Engine/Core/Updatable.h"

class PongPlatform;

namespace sf {
class Event;
}

class PlatformControllerBase : public Updatable
{
public:
	explicit PlatformControllerBase(PongPlatform* platform) : _platform(platform) {}

	virtual void Init() {}

protected:
	PongPlatform* _platform = nullptr;
};
