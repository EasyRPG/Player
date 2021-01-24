#.rst:
# FindPixman
# -----------
#
# Find the Pixman Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``PIXMAN::PIXMAN``
#   The ``Pixman`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``PIXMAN_INCLUDE_DIRS``
#   where to find Pixman headers.
# ``PIXMAN_LIBRARIES``
#   the libraries to link against to use Pixman.
# ``PIXMAN_FOUND``
#   true if the Pixman headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_PIXMAN QUIET pixman-1)

# Look for the header file.
find_path(PIXMAN_INCLUDE_DIR
	NAMES pixman.h
	PATH_SUFFIXES libpixman-1 pixman-1
	HINTS ${PC_PIXMAN_INCLUDE_DIRS})

# Look for the library.
# Allow PIXMAN_LIBRARY to be set manually, as the location of the Pixman library
if(NOT PIXMAN_LIBRARY)
	find_library(PIXMAN_LIBRARY_RELEASE
		NAMES libpixman-1 pixman-1
		HINTS ${PC_PIXMAN_LIBRARY_DIRS})

	find_library(PIXMAN_LIBRARY_DEBUG
		NAMES libpixman-1d pixman-1d
		HINTS ${PC_PIXMAN_LIBRARY_DIRS})

	include(SelectLibraryConfigurations)
	select_library_configurations(PIXMAN)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pixman
	REQUIRED_VARS PIXMAN_LIBRARY PIXMAN_INCLUDE_DIR)

if(ANDROID)
	enable_language(C)
	# armeabi-v7a requires this
	add_library(cpufeatures STATIC
		${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
endif()

if(PIXMAN_FOUND)
	set(PIXMAN_INCLUDE_DIRS ${PIXMAN_INCLUDE_DIR})

	if(NOT PIXMAN_LIBRARIES)
		set(PIXMAN_LIBRARIES ${PIXMAN_LIBRARIES})
	endif()

	if(NOT TARGET PIXMAN::PIXMAN)
		add_library(PIXMAN::PIXMAN UNKNOWN IMPORTED)
		set_target_properties(PIXMAN::PIXMAN PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${PIXMAN_INCLUDE_DIRS}")

		if(PIXMAN_LIBRARY_RELEASE)
			set_property(TARGET PIXMAN::PIXMAN APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE)
			set_target_properties(PIXMAN::PIXMAN PROPERTIES
				IMPORTED_LOCATION_RELEASE "${PIXMAN_LIBRARY_RELEASE}")
		endif()

		if(PIXMAN_LIBRARY_DEBUG)
			set_property(TARGET PIXMAN::PIXMAN APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG)
			set_target_properties(PIXMAN::PIXMAN PROPERTIES
				IMPORTED_LOCATION_DEBUG "${PIXMAN_LIBRARY_DEBUG}")
		endif()

		if(NOT PIXMAN_LIBRARY_RELEASE AND NOT PIXMAN_LIBRARY_DEBUG)
			set_property(TARGET PIXMAN::PIXMAN APPEND PROPERTY
				IMPORTED_LOCATION "${PIXMAN_LIBRARY}")
		endif()

		if(ANDROID)
			set_property(TARGET PIXMAN::PIXMAN APPEND PROPERTY
				INTERFACE_LINK_LIBRARIES cpufeatures)
		endif()
	endif()
endif()

mark_as_advanced(PIXMAN_INCLUDE_DIR PIXMAN_LIBRARY)
