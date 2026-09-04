# FMOD Engine (Studio API for Windows). Not FMOD Studio authoring.
# Expected layout: <FMOD_DIR>/api/core/{inc,lib/x64} and <FMOD_DIR>/api/studio/{inc,lib/x64}

set(_FMOD_DEFAULT_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows")

if(NOT FMOD_DIR)
	if(DEFINED ENV{FMOD_DIR} AND NOT "$ENV{FMOD_DIR}" STREQUAL "")
		set(FMOD_DIR "$ENV{FMOD_DIR}")
	else()
		set(FMOD_DIR "${_FMOD_DEFAULT_DIR}")
	endif()
endif()
set(FMOD_DIR "${FMOD_DIR}" CACHE PATH "FMOD Engine (Studio API Windows) install directory")

set(_FMOD_ARCH "x64")
set(_FMOD_CORE_INC "${FMOD_DIR}/api/core/inc")
set(_FMOD_STUDIO_INC "${FMOD_DIR}/api/studio/inc")
set(_FMOD_CORE_LIBDIR "${FMOD_DIR}/api/core/lib/${_FMOD_ARCH}")
set(_FMOD_STUDIO_LIBDIR "${FMOD_DIR}/api/studio/lib/${_FMOD_ARCH}")

find_path(FMOD_CORE_INCLUDE_DIR NAMES fmod.hpp HINTS "${_FMOD_CORE_INC}" NO_DEFAULT_PATH)
find_path(FMOD_STUDIO_INCLUDE_DIR NAMES fmod_studio.hpp HINTS "${_FMOD_STUDIO_INC}" NO_DEFAULT_PATH)

find_library(FMOD_CORE_LIBRARY_RELEASE NAMES fmod_vc HINTS "${_FMOD_CORE_LIBDIR}" NO_DEFAULT_PATH)
find_library(FMOD_CORE_LIBRARY_DEBUG NAMES fmodL_vc HINTS "${_FMOD_CORE_LIBDIR}" NO_DEFAULT_PATH)
find_library(FMOD_STUDIO_LIBRARY_RELEASE NAMES fmodstudio_vc HINTS "${_FMOD_STUDIO_LIBDIR}" NO_DEFAULT_PATH)
find_library(FMOD_STUDIO_LIBRARY_DEBUG NAMES fmodstudioL_vc HINTS "${_FMOD_STUDIO_LIBDIR}" NO_DEFAULT_PATH)

find_file(FMOD_CORE_DLL_RELEASE NAMES fmod.dll HINTS "${_FMOD_CORE_LIBDIR}" NO_DEFAULT_PATH)
find_file(FMOD_CORE_DLL_DEBUG NAMES fmodL.dll HINTS "${_FMOD_CORE_LIBDIR}" NO_DEFAULT_PATH)
find_file(FMOD_STUDIO_DLL_RELEASE NAMES fmodstudio.dll HINTS "${_FMOD_STUDIO_LIBDIR}" NO_DEFAULT_PATH)
find_file(FMOD_STUDIO_DLL_DEBUG NAMES fmodstudioL.dll HINTS "${_FMOD_STUDIO_LIBDIR}" NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	FMOD
	REQUIRED_VARS
		FMOD_CORE_INCLUDE_DIR
		FMOD_STUDIO_INCLUDE_DIR
		FMOD_CORE_LIBRARY_RELEASE
		FMOD_CORE_LIBRARY_DEBUG
		FMOD_STUDIO_LIBRARY_RELEASE
		FMOD_STUDIO_LIBRARY_DEBUG
		FMOD_CORE_DLL_RELEASE
		FMOD_CORE_DLL_DEBUG
		FMOD_STUDIO_DLL_RELEASE
		FMOD_STUDIO_DLL_DEBUG
	FAIL_MESSAGE
		"FMOD Engine (Studio API Windows) not found. Install it from https://www.fmod.com/download (not FMOD Studio authoring) and set FMOD_DIR to the install root, e.g. C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows"
)

if(NOT FMOD_FOUND)
	return()
endif()

function(_testengine_fmod_imported_shared target_name include_dir implib_rel implib_dbg dll_rel dll_dbg)
	if(TARGET ${target_name})
		return()
	endif()
	add_library(${target_name} SHARED IMPORTED)
	set_target_properties(
		${target_name}
		PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${include_dir}"
			IMPORTED_CONFIGURATIONS "DEBUG;RELEASE;RELWITHDEBINFO;MINSIZEREL"
			IMPORTED_IMPLIB "${implib_rel}"
			IMPORTED_LOCATION "${dll_rel}"
			IMPORTED_IMPLIB_RELEASE "${implib_rel}"
			IMPORTED_LOCATION_RELEASE "${dll_rel}"
			IMPORTED_IMPLIB_MINSIZEREL "${implib_rel}"
			IMPORTED_LOCATION_MINSIZEREL "${dll_rel}"
			IMPORTED_IMPLIB_DEBUG "${implib_dbg}"
			IMPORTED_LOCATION_DEBUG "${dll_dbg}"
			IMPORTED_IMPLIB_RELWITHDEBINFO "${implib_dbg}"
			IMPORTED_LOCATION_RELWITHDEBINFO "${dll_dbg}"
	)
endfunction()

_testengine_fmod_imported_shared(
	FMOD::Core
	"${FMOD_CORE_INCLUDE_DIR}"
	"${FMOD_CORE_LIBRARY_RELEASE}"
	"${FMOD_CORE_LIBRARY_DEBUG}"
	"${FMOD_CORE_DLL_RELEASE}"
	"${FMOD_CORE_DLL_DEBUG}"
)
_testengine_fmod_imported_shared(
	FMOD::Studio
	"${FMOD_STUDIO_INCLUDE_DIR}"
	"${FMOD_STUDIO_LIBRARY_RELEASE}"
	"${FMOD_STUDIO_LIBRARY_DEBUG}"
	"${FMOD_STUDIO_DLL_RELEASE}"
	"${FMOD_STUDIO_DLL_DEBUG}"
)
set_property(TARGET FMOD::Studio PROPERTY INTERFACE_LINK_LIBRARIES FMOD::Core)

mark_as_advanced(
	FMOD_CORE_INCLUDE_DIR
	FMOD_STUDIO_INCLUDE_DIR
	FMOD_CORE_LIBRARY_RELEASE
	FMOD_CORE_LIBRARY_DEBUG
	FMOD_STUDIO_LIBRARY_RELEASE
	FMOD_STUDIO_LIBRARY_DEBUG
	FMOD_CORE_DLL_RELEASE
	FMOD_CORE_DLL_DEBUG
	FMOD_STUDIO_DLL_RELEASE
	FMOD_STUDIO_DLL_DEBUG
)
