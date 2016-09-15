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

set(HARFBUZZ_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIR})
set(HARFBUZZ_LIBRARIES ${HARFBUZZ_LIBRARY})

mark_as_advanced(HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)
