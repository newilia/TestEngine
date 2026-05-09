#pragma once

#include "Engine/Editor/PropertyTreeDrawer.h"

namespace Engine {

	/// Inspector-style editing for `GameBackgroundContext` (type combo + reflected properties).
	class GameBackgroundWidget
	{
	public:
		void Draw() const;

	private:
		PropertyTreeDrawer _propertyDrawer{};
	};

} // namespace Engine
