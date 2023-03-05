#.rst:
# FindFluidSynth
# -----------
#
# Find the FluidSynth Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``FluidSynth::libfluidsynth``
#   The ``FluidSynth`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FLUIDSYNTH_INCLUDE_DIRS``
#   where to find FluidSynth headers.
# ``FLUIDSYNTH_LIBRARIES``
#   the libraries to link against to use FluidSynth.
# ``FLUIDSYNTH_FOUND``
#   true if the FluidSynth headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_FLUIDSYNTH QUIET fluidsynth)

# Look for the header file.
find_path(FLUIDSYNTH_INCLUDE_DIR
	NAMES fluidsynth.h
	PATH_SUFFIXES libfluidsynth fluidsynth
	HINTS ${PC_FLUIDSYNTH_INCLUDE_DIRS})

# Look for the library.
# Allow FLUIDSYNTH_LIBRARY to be set manually, as the location of the FluidSynth library
if(NOT FLUIDSYNTH_LIBRARY)
	find_library(FLUIDSYNTH_LIBRARY
		NAMES libfluidsynth fluidsynth
		HINTS ${PC_FLUIDSYNTH_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FluidSynth
	REQUIRED_VARS FLUIDSYNTH_LIBRARY FLUIDSYNTH_INCLUDE_DIR)

if(FLUIDSYNTH_FOUND)
	set(FLUIDSYNTH_INCLUDE_DIRS ${FLUIDSYNTH_INCLUDE_DIR})

	if(NOT FLUIDSYNTH_LIBRARIES)
		set(FLUIDSYNTH_LIBRARIES ${FLUIDSYNTH_LIBRARIES})
	endif()

	if(NOT TARGET FluidSynth::libfluidsynth)
		add_library(FluidSynth::libfluidsynth UNKNOWN IMPORTED)
		set_target_properties(FluidSynth::libfluidsynth PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FLUIDSYNTH_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${FLUIDSYNTH_LIBRARY}")
	endif()
endif()

mark_as_advanced(FLUIDSYNTH_INCLUDE_DIR FLUIDSYNTH_LIBRARY)
