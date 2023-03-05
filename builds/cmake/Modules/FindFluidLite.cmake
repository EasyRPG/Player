#.rst:
# FindFluidLite
# -----------
#
# Find the FluidLite Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``FluidLite::fluidlite``
#   The ``FluidLite`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FLUIDLITE_INCLUDE_DIRS``
#   where to find FluidLite headers.
# ``FLUIDLITE_LIBRARIES``
#   the libraries to link against to use FluidLite.
# ``FLUIDLITE_FOUND``
#   true if the FluidLite headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_FLUIDLITE QUIET fluidlite)

# Look for the header file.
find_path(FLUIDLITE_INCLUDE_DIR
	NAMES fluidlite.h
	PATH_SUFFIXES libfluidlite fluidlite
	HINTS ${PC_FLUIDLITE_INCLUDE_DIRS})

# Look for the library.
# Allow FLUIDLITE_LIBRARY to be set manually, as the location of the FluidLite library
if(NOT FLUIDLITE_LIBRARY)
	find_library(FLUIDLITE_LIBRARY
		NAMES libfluidlite fluidlite
		HINTS ${PC_FLUIDLITE_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FluidLite
	REQUIRED_VARS FLUIDLITE_LIBRARY FLUIDLITE_INCLUDE_DIR)

if(FLUIDLITE_FOUND)
	set(FLUIDLITE_INCLUDE_DIRS ${FLUIDLITE_INCLUDE_DIR})

	if(NOT FLUIDLITE_LIBRARIES)
		set(FLUIDLITE_LIBRARIES ${FLUIDLITE_LIBRARIES})
	endif()

	if(NOT TARGET FluidLite::fluidlite)
		add_library(FluidLite::fluidlite UNKNOWN IMPORTED)
		set_target_properties(FluidLite::fluidlite PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FLUIDLITE_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${FLUIDLITE_LIBRARY}")
	endif()
endif()

mark_as_advanced(FLUIDLITE_INCLUDE_DIR FLUIDLITE_LIBRARY)
