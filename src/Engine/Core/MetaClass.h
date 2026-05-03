#pragma once

namespace Engine {
	class PropertyBuilder;
}

/// Declares `BuildPropertyTree` for classes that use `tools/property_codegen.py` and a matching `*.generated.hpp`.
#define META_CLASS()                                                                                                   \
public:                                                                                                                \
	void BuildPropertyTree(::Engine::PropertyBuilder& builder) override;

/// Optional: immediate base whose generated `BuildPropertyTree` runs first for this class (read by `tools/property_codegen.py` only).
#define META_PROPERTY_BASE(Base)
