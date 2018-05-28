#.rst:
# FindHarfbuzz
# -----------
#
# Find the Harfbuzz Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Harfbuzz::Harfbuzz``
#   The ``Harfbuzz`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``HARFBUZZ_INCLUDE_DIRS``
#   where to find Harfbuzz headers.
# ``HARFBUZZ_LIBRARIES``
#   the libraries to link against to use Harfbuzz.
# ``HARFBUZZ_FOUND``
#   true if the Harfbuzz headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_HARFBUZZ QUIET harfbuzz)

# Look for the header file.
find_path(HARFBUZZ_INCLUDE_DIR
	NAMES hb.h
	PATH_SUFFIXES libharfbuzz harfbuzz
	HINTS ${PC_HARFBUZZ_INCLUDE_DIRS})

# Look for the library.
# Allow HARFBUZZ_LIBRARY to be set manually, as the location of the Harfbuzz library
if(NOT HARFBUZZ_LIBRARY)
	find_library(HARFBUZZ_LIBRARY
		NAMES libharfbuzz harfbuzz
		HINTS ${PC_HARFBUZZ_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Harfbuzz
	REQUIRED_VARS HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)

if(HARFBUZZ_FOUND)
	set(HARFBUZZ_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIR})

	if(NOT HARFBUZZ_LIBRARIES)
		set(HARFBUZZ_LIBRARIES ${HARFBUZZ_LIBRARIES})
	endif()

	if(NOT TARGET Harfbuzz::Harfbuzz)
		add_library(Harfbuzz::Harfbuzz UNKNOWN IMPORTED)
		find_library(Freetype REQUIRED)
		set_target_properties(Harfbuzz::Harfbuzz PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${HARFBUZZ_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES Freetype::Freetype
			IMPORTED_LOCATION "${HARFBUZZ_LIBRARY}")
	endif()
endif()

mark_as_advanced(HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)
