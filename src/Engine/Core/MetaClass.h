#pragma once

namespace Engine {
	class PropertyBuilder;
}

/// Declares `BuildPropertyTree` for classes that use `tools/property_codegen.py` and a matching `*.generated.hpp`.
#define META_CLASS()                                                                                                   \
public:                                                                                                                \
	void BuildPropertyTree(::Engine::PropertyBuilder& builder) override;
