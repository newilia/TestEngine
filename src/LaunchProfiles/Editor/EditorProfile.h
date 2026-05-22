#pragma once

#include "LaunchProfiles/LaunchProfileBase.h"

class EditorProfile : public LaunchProfileBase
{
public:
	~EditorProfile() override = default;
	void Setup() override;
};
