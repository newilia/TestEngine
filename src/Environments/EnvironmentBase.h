#pragma once

class EnvironmentBase
{
public:
	virtual ~EnvironmentBase() = default;
	virtual void Setup() = 0;
};
