#.rst:
# FindTremor
# -----------
#
# Find the Tremor Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Tremor::Tremor``
#   The ``Tremor`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``TREMOR_INCLUDE_DIRS``
#   where to find Tremor headers.
# ``TREMOR_LIBRARIES``
#   the libraries to link against to use Tremor.
# ``TREMOR_FOUND``
#   true if the Tremor headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_TREMOR QUIET tremor)

# Look for the header file.
find_path(TREMOR_INCLUDE_DIR
	NAMES ogg.h
	PATH_SUFFIXES libogg ogg
	HINTS ${PC_TREMOR_INCLUDE_DIRS})

# Look for the library.
# Allow TREMOR_LIBRARY to be set manually, as the location of the Tremor library
if(NOT TREMOR_LIBRARY)
	find_library(TREMOR_LIBRARY
		NAMES libvorbisidec vorbisidec
		HINTS ${PC_TREMOR_LIBRARY_DIRS})
endif()

# Additional dependencies
find_library(OGG_LIBRARY
        NAMES libogg ogg)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tremor
	REQUIRED_VARS TREMOR_LIBRARY TREMOR_INCLUDE_DIR
	VERSION_VAR TREMOR_VERSION)

if(TREMOR_FOUND)
	set(TREMOR_INCLUDE_DIRS ${TREMOR_INCLUDE_DIR})

	if(NOT TREMOR_LIBRARIES)
		set(TREMOR_LIBRARIES ${TREMOR_LIBRARIES})
	endif()

	if(NOT TARGET Tremor::Tremor)
		add_library(Tremor::Tremor UNKNOWN IMPORTED)
		set_target_properties(Tremor::Tremor PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${TREMOR_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${OGG_LIBRARY}"
			IMPORTED_LOCATION "${TREMOR_LIBRARY}")
	endif()
endif()

mark_as_advanced(TREMOR_INCLUDE_DIR TREMOR_LIBRARY)
