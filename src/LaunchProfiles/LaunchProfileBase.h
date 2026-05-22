#pragma once

class LaunchProfileBase
{
public:
	virtual ~LaunchProfileBase() = default;
	virtual void Setup() = 0;
};
