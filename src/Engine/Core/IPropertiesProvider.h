#pragma once

namespace Engine {

	class PropertyBuilder;

	/// Optional reflection hook for editor and tools. Default: no properties.

	struct IPropertiesProvider
	{
		virtual ~IPropertiesProvider() = default;

		virtual void BuildPropertyTree(PropertyBuilder& builder);
	};

} // namespace Engine
