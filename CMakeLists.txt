cmake_minimum_required(VERSION 3.28)

# MSVC: honor MSVC_DEBUG_INFORMATION_FORMAT (Embedded /Z7 vs ProgramDatabase) in generated flags.
if(POLICY CMP0091)
	cmake_policy(SET CMP0091 NEW)
endif()

project(TestEngine LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/src/Codegen")

add_custom_target(
	Codegen
	COMMAND Python3::Interpreter "${CMAKE_SOURCE_DIR}/tools/property_codegen.py" --root "${CMAKE_SOURCE_DIR}"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Running tools/property_codegen.py"
	VERBATIM
)

add_custom_target(
	Codegen_Force
	COMMAND Python3::Interpreter "${CMAKE_SOURCE_DIR}/tools/property_codegen.py" --root "${CMAKE_SOURCE_DIR}" --force
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Running tools/property_codegen.py --force"
	VERBATIM
)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/**.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/**.h")
file(GLOB_RECURSE ENGINE_CODEGEN CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/Codegen/*_gen.hpp")

if(NOT ENGINE_SOURCES)
	message(FATAL_ERROR "No source files found in src/. Check project structure.")
endif()

add_executable(TestEngine WIN32 ${ENGINE_SOURCES} ${ENGINE_HEADERS} ${ENGINE_CODEGEN})

if(ENGINE_HEADERS)
	set_source_files_properties(${ENGINE_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
if(ENGINE_CODEGEN)
	set_source_files_properties(${ENGINE_CODEGEN} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

add_dependencies(TestEngine Codegen)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "" FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Codegen" PREFIX "Codegen" FILES ${ENGINE_CODEGEN})

if(MSVC)
	target_compile_options(TestEngine PRIVATE /FS)
	# Debug: /Z7 embeds symbols in .obj and avoids C1041 (parallel cl.exe vs one vc*.pdb) when /FS alone is not enough.
	target_compile_options(TestEngine PRIVATE $<$<CONFIG:Debug>:/Z7>)
	# Do not run C++ Core Check inside vendored fmt headers (we do not patch third-party/fmt).
	target_compile_options(
		TestEngine
		PRIVATE
			"/external:I${CMAKE_SOURCE_DIR}/third-party/fmt/include"
			/analyze:external-
	)
endif()

target_include_directories(
	TestEngine
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/Codegen
)

# fmt submodule: third-party/fmt
add_subdirectory(third-party/fmt EXCLUDE_FROM_ALL)
target_link_libraries(TestEngine PRIVATE fmt)

# SFML submodule: third-party/SFML
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/SFML EXCLUDE_FROM_ALL)

# Dear ImGui + ImGui-SFML (submodules under third-party/)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third-party/imgui" CACHE PATH "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/imgui-sfml EXCLUDE_FROM_ALL)

target_link_libraries(TestEngine PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
    ImGui-SFML::ImGui-SFML
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

