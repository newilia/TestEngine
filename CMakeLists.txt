cmake_minimum_required(VERSION 3.16)

# MSVC: honor MSVC_DEBUG_INFORMATION_FORMAT (Embedded /Z7 vs ProgramDatabase) in generated flags.
if(POLICY CMP0091)
	cmake_policy(SET CMP0091 NEW)
endif()

project(TestEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE ENGINE_SOURCES
    "src/**.cpp"
)

if(NOT ENGINE_SOURCES)
    message(FATAL_ERROR "No source files found in src/. Check project structure.")
endif()

add_executable(TestEngine WIN32 ${ENGINE_SOURCES})

if(MSVC)
	target_compile_options(TestEngine PRIVATE /FS)
	# Debug: /Z7 embeds symbols in .obj and avoids C1041 (parallel cl.exe vs one vc*.pdb) when /FS alone is not enough.
	target_compile_options(TestEngine PRIVATE $<$<CONFIG:Debug>:/Z7>)
endif()

target_include_directories(TestEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# fmt submodule: third-party/fmt
add_subdirectory(third-party/fmt EXCLUDE_FROM_ALL)
target_link_libraries(TestEngine PRIVATE fmt)

# SFML submodule: third-party/SFML
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/SFML EXCLUDE_FROM_ALL)

target_link_libraries(TestEngine PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)

# Стартовый проект в Visual Studio (F5) — иначе часто остаётся ALL_BUILD / таргет из third-party
set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT TestEngine)

# При запуске/отладке из Visual Studio (и CMake в VS Code) CWD = корень репозитория
set_target_properties(
	TestEngine
	PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		MSVC_DEBUG_INFORMATION_FORMAT
		"$<$<CONFIG:Debug>:Embedded>$<$<NOT:$<CONFIG:Debug>>:ProgramDatabase>"
)

