#.rst:
# FindSamplerate
# -----------
#
# Find the Samplerate Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Samplerate::Samplerate``
#   The ``Samplerate`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``SAMPLERATE_INCLUDE_DIRS``
#   where to find Samplerate headers.
# ``SAMPLERATE_LIBRARIES``
#   the libraries to link against to use Samplerate.
# ``SAMPLERATE_FOUND``
#   true if the Samplerate headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_SAMPLERATE QUIET samplerate)

# Look for the header file.
find_path(SAMPLERATE_INCLUDE_DIR
	NAMES samplerate.h
	PATH_SUFFIXES libsamplerate samplerate
	HINTS ${PC_SAMPLERATE_INCLUDE_DIRS})

# Look for the library.
# Allow SAMPLERATE_LIBRARY to be set manually, as the location of the Samplerate library
if(NOT SAMPLERATE_LIBRARY)
	find_library(SAMPLERATE_LIBRARY
		NAMES libsamplerate samplerate
		HINTS ${PC_SAMPLERATE_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Samplerate
	REQUIRED_VARS SAMPLERATE_LIBRARY SAMPLERATE_INCLUDE_DIR)

if(SAMPLERATE_FOUND)
	set(SAMPLERATE_INCLUDE_DIRS ${SAMPLERATE_INCLUDE_DIR})

	if(NOT SAMPLERATE_LIBRARIES)
		set(SAMPLERATE_LIBRARIES ${SAMPLERATE_LIBRARIES})
	endif()

	if(NOT TARGET Samplerate::Samplerate)
		add_library(Samplerate::Samplerate UNKNOWN IMPORTED)
		set_target_properties(Samplerate::Samplerate PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SAMPLERATE_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${SAMPLERATE_LIBRARY}")
	endif()
endif()

mark_as_advanced(SAMPLERATE_INCLUDE_DIR SAMPLERATE_LIBRARY)
