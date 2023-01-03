#.rst:
# Findmpg123
# -----------
#
# Find the mpg123 Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``MPG123::libmpg123``
#   The ``mpg123`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``MPG123_INCLUDE_DIRS``
#   where to find mpg123 headers.
# ``MPG123_LIBRARIES``
#   the libraries to link against to use mpg123.
# ``MPG123_FOUND``
#   true if the mpg123 headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_MPG123 QUIET libmpg123)

# Look for the header file.
find_path(MPG123_INCLUDE_DIR
	NAMES mpg123.h
	PATH_SUFFIXES libmpg123 mpg123
	HINTS ${PC_MPG123_INCLUDE_DIRS})

# Look for the library.
# Allow MPG123_LIBRARY to be set manually, as the location of the mpg123 library
if(NOT MPG123_LIBRARY)
	find_library(MPG123_LIBRARY
		NAMES libmpg123 mpg123
		HINTS ${PC_MPG123_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpg123
	REQUIRED_VARS MPG123_LIBRARY MPG123_INCLUDE_DIR)

if(MPG123_FOUND)
	set(MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIR})

	if(NOT MPG123_LIBRARIES)
		set(MPG123_LIBRARIES ${MPG123_LIBRARIES})
	endif()

	if(NOT TARGET MPG123::libmpg123)
		add_library(MPG123::libmpg123 UNKNOWN IMPORTED)
		set_target_properties(MPG123::libmpg123 PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${MPG123_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${MPG123_LIBRARY}")
		if(WIN32)
			set_property(TARGET MPG123::libmpg123 APPEND_STRING PROPERTY
				INTERFACE_LINK_LIBRARIES "shlwapi")
		endif()
	endif()
endif()

mark_as_advanced(MPG123_INCLUDE_DIR MPG123_LIBRARY)
