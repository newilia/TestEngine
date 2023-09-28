#pragma once
#include "Updateable.h"

class PongPlatform;

namespace sf {
	class Event;
}

class PlatformControllerBase : public Updateable {
public:
	explicit PlatformControllerBase(PongPlatform* platform) : mPlatform(platform) {}
	virtual void init() {}
protected:
	PongPlatform* mPlatform = nullptr;
};
