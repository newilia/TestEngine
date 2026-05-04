# Build-time driver for tools/property_codegen.py (invoked via: cmake -P ... -DROOT=... -DPython3_EXECUTABLE=...).
# Ensures a non-zero exit code on failure for all generators (including MSVC).
if(NOT DEFINED ROOT OR NOT DEFINED Python3_EXECUTABLE)
	message(FATAL_ERROR "RunPropertyCodegen.cmake: pass -DROOT=... and -DPython3_EXECUTABLE=...")
endif()

set(_script "${ROOT}/tools/property_codegen.py")
if(NOT EXISTS "${_script}")
	message(FATAL_ERROR "RunPropertyCodegen.cmake: missing script: ${_script}")
endif()

if(FORCE)
	execute_process(
		COMMAND "${Python3_EXECUTABLE}" "${_script}" --root "${ROOT}" --force
		WORKING_DIRECTORY "${ROOT}"
		RESULT_VARIABLE _rv
	)
else()
	execute_process(
		COMMAND "${Python3_EXECUTABLE}" "${_script}" --root "${ROOT}"
		WORKING_DIRECTORY "${ROOT}"
		RESULT_VARIABLE _rv
	)
endif()

if(NOT _rv EQUAL 0)
	message(FATAL_ERROR "property_codegen.py failed (exit code: ${_rv})")
endif()
