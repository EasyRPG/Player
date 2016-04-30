# HARFBUZZ_INCLUDE_DIR - harfbuzz include directory
# Harfbuzz_FOUND - wether harfbuzz is found
# HARFBUZZ_LIBRARY - harfbuzz library

include(FindPackageHandleStandardArgs)

find_path(HARFBUZZ_INCLUDE_DIR_INTERNAL harfbuzz/hb.h)
find_library(HARFBUZZ_LIBRARY harfbuzz)
if(EXISTS "${HARFBUZZ_INCLUDE_DIR_INTERNAL}")
  set(HARFBUZZ_INCLUDE_DIR "${HARFBUZZ_INCLUDE_DIR_INTERNAL}/harfbuzz")
endif()

find_package_handle_standard_args(Harfbuzz
	REQUIRED_VARS HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)
