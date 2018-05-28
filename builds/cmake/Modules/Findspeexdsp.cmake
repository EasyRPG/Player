#.rst:
# Findspeexdsp
# -----------
#
# Find the speexdsp Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``speexdsp::speexdsp``
#   The ``speexdsp`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``SPEEXDSP_INCLUDE_DIRS``
#   where to find speexdsp headers.
# ``SPEEXDSP_LIBRARIES``
#   the libraries to link against to use speexdsp.
# ``SPEEXDSP_FOUND``
#   true if the speexdsp headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_SPEEXDSP QUIET speexdsp)

# Look for the header file.
find_path(SPEEXDSP_INCLUDE_DIR
	NAMES speex_resampler.h
	PATH_SUFFIXES speex
	HINTS ${PC_SPEEXDSP_INCLUDE_DIRS})

# Look for the library.
# Allow SPEEXDSP_LIBRARY to be set manually, as the location of the speexdsp library
if(NOT SPEEXDSP_LIBRARY)
	find_library(SPEEXDSP_LIBRARY
		NAMES libspeexdsp speexdsp
		HINTS ${PC_SPEEXDSP_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(speexdsp
	REQUIRED_VARS SPEEXDSP_LIBRARY SPEEXDSP_INCLUDE_DIR)

if(SPEEXDSP_FOUND)
	set(SPEEXDSP_INCLUDE_DIRS ${SPEEXDSP_INCLUDE_DIR})

	if(NOT SPEEXDSP_LIBRARIES)
		set(SPEEXDSP_LIBRARIES ${SPEEXDSP_LIBRARIES})
	endif()

	if(NOT TARGET speexdsp::speexdsp)
		add_library(speexdsp::speexdsp UNKNOWN IMPORTED)
		set_target_properties(speexdsp::speexdsp PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SPEEXDSP_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${SPEEXDSP_LIBRARY}")
	endif()
endif()

mark_as_advanced(SPEEXDSP_INCLUDE_DIR SPEEXDSP_LIBRARY)
