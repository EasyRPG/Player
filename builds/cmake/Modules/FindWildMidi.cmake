#.rst:
# FindWildMidi
# -----------
#
# Find the WildMidi Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``WildMidi::WildMidi``
#   The ``WildMidi`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``WILDMIDI_INCLUDE_DIRS``
#   where to find WildMidi headers.
# ``WILDMIDI_LIBRARIES``
#   the libraries to link against to use WildMidi.
# ``WILDMIDI_FOUND``
#   true if the WildMidi headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_WILDMIDI QUIET wildmidi)

# Look for the header file.
find_path(WILDMIDI_INCLUDE_DIR
	NAMES wildmidi_lib.h
	PATH_SUFFIXES libwildmidi wildmidi
	HINTS ${PC_WILDMIDI_INCLUDE_DIRS})

# Look for the library.
# Allow WILDMIDI_LIBRARY to be set manually, as the location of the WildMidi library
if(NOT WILDMIDI_LIBRARY)
	find_library(WILDMIDI_LIBRARY
		NAMES libwildmidi wildmidi libWildMidi WildMidi
		HINTS ${PC_WILDMIDI_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WildMidi
	REQUIRED_VARS WILDMIDI_LIBRARY WILDMIDI_INCLUDE_DIR)

if(WILDMIDI_FOUND)
	set(WILDMIDI_INCLUDE_DIRS ${WILDMIDI_INCLUDE_DIR})

	if(NOT WILDMIDI_LIBRARIES)
		set(WILDMIDI_LIBRARIES ${WILDMIDI_LIBRARIES})
	endif()

	if(NOT TARGET WildMidi::WildMidi)
		add_library(WildMidi::WildMidi UNKNOWN IMPORTED)
		set_target_properties(WildMidi::WildMidi PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${WILDMIDI_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${WILDMIDI_LIBRARY}")
		if(WIN32)
			set_target_properties(WildMidi::WildMidi PROPERTIES
				INTERFACE_COMPILE_DEFINITIONS "WILDMIDI_STATIC=1")
		endif()
	endif()
endif()

mark_as_advanced(WILDMIDI_INCLUDE_DIR WILDMIDI_LIBRARY)
