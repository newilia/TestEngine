#pragma once

#include "Engine/Core/MetaClass.h"

#include <string>

namespace Billiard {

	META_ENUM(PlayerKind, LocalHuman, RemoteHuman, Ai);

	struct PlayerSlotConfig
	{
		PlayerKind kind = PlayerKind::LocalHuman;
		std::string displayName;
	};

} // namespace Billiard
