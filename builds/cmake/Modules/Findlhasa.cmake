#.rst:
# Findlhasa
# -----------
#
# Find the lhasa Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``LHASA::liblhasa``
#   The ``lhasa`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``LHASA_INCLUDE_DIRS``
#   where to find lhasa headers.
# ``LHASA_LIBRARIES``
#   the libraries to link against to use lhasa.
# ``LHASA_FOUND``
#   true if the lhasa headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_LHASA QUIET liblhasa)

# Look for the header file.
find_path(LHASA_INCLUDE_DIR
	NAMES lhasa.h
	PATH_SUFFIXES liblhasa-1.0 liblhasa
	HINTS ${PC_LHASA_INCLUDE_DIRS})

# Look for the library.
# Allow LHASA_LIBRARY to be set manually, as the location of the lhasa library
if(NOT LHASA_LIBRARY)
	find_library(LHASA_LIBRARY
		NAMES liblhasa lhasa
		HINTS ${PC_LHASA_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lhasa
	REQUIRED_VARS LHASA_LIBRARY LHASA_INCLUDE_DIR)

if(LHASA_FOUND)
	set(LHASA_INCLUDE_DIRS ${LHASA_INCLUDE_DIR})

	if(NOT LHASA_LIBRARIES)
		set(LHASA_LIBRARIES ${LHASA_LIBRARIES})
	endif()

	if(NOT TARGET LHASA::liblhasa)
		add_library(LHASA::liblhasa UNKNOWN IMPORTED)
		set_target_properties(LHASA::liblhasa PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${LHASA_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${LHASA_LIBRARY}")
	endif()
endif()

mark_as_advanced(LHASA_INCLUDE_DIR LHASA_LIBRARY)
