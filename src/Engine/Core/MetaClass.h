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

/// Declares `enum class Name { ... }` and registers enumerators for `tools/property_codegen.py` (must end with `;` on the same line).
#define META_ENUM(Name, ...)                                                                                           \
	enum class Name                                                                                                    \
	{                                                                                                                  \
		__VA_ARGS__                                                                                                    \
	}
