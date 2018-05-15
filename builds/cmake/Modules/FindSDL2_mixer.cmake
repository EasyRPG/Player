#.rst:
# FindSDL2_mixer
# -----------
#
# Find the SDL2_mixer Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``SDL2::MIXER``
#   The ``SDL2_mixer`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``SDL2_MIXER_INCLUDE_DIRS``
#   where to find SDL2_mixer headers.
# ``SDL2_MIXER_LIBRARIES``
#   the libraries to link against to use SDL2_mixer.
# ``SDL2_MIXER_FOUND``
#   true if the SDL2_mixer headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)

# Look for the header file.
find_path(SDL2_MIXER_INCLUDE_DIR
	NAMES SDL_mixer.h
	PATH_SUFFIXES include/SDL2
	HINTS ${PC_SDL2_MIXER_INCLUDE_DIRS})

# Look for the library.
# Allow SDL2_MIXER_LIBRARY to be set manually, as the location of the SDL2_mixer library
if(NOT SDL2_MIXER_LIBRARY)
	find_library(SDL2_MIXER_LIBRARY
		NAMES libSDL2_mixer SDL2_mixer
		HINTS ${PC_SDL2_MIXER_LIBRARY_DIRS})
endif()

set(SDL2_MIXER_EXTRA_DEPS "")

# Find additional dependencies
find_library(VORBIS_LIBRARY
	NAMES libvorbis vorbis
	HINTS ${PC_SDL2_MIXER_LIBRARY_DIRS})
if(VORBIS_LIBRARY_FOUND)
	# Required dependency of vorbis
	find_library(OGG_LIBRARY
		NAMES libogg ogg
		HINTS ${PC_SDL2_MIXER_LIBRARY_DIRS})

	list(APPEND SDL2_MIXER_EXTRA_DEPS ${VORBIS_LIBRARY} ${OGG_LIBRARY})
endif()

find_package(mpg123 QUIET)
if(MPG123_FOUND)
	list(APPEND SDL2_MIXER_EXTRA_DEPS mpg123::mpg123)
endif()

find_library(FLAC_LIBRARY
	NAMES libFLAC FLAC
	HINTS ${PC_SDL2_MIXER_LIBRARY_DIRS})
if(FLAC_LIBRARY_FOUND)
	list(APPEND SDL2_MIXER_EXTRA_DEPS ${FLAC_LIBRARY})
endif()

find_library(MODPLUG_LIBRARY
	NAMES libmodplug modplug
	HINTS ${PC_SDL2_MIXER_LIBRARY_DIRS})
if(MODPLUG_LIBRARY_FOUND)
	list(APPEND SDL2_MIXER_EXTRA_DEPS ${MODPLUG_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer
	REQUIRED_VARS SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)

if(SDL2_MIXER_FOUND)
	set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

	if(NOT SDL2_MIXER_LIBRARIES)
		set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARIES})
	endif()

	if(NOT TARGET SDL2::MIXER)
		add_library(SDL2::MIXER UNKNOWN IMPORTED)
		set_target_properties(SDL2::MIXER PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${SDL2_MIXER_EXTRA_DEPS}"
			IMPORTED_LOCATION "${SDL2_MIXER_LIBRARY}")
	endif()
endif()

mark_as_advanced(SDL2_MIXER_INCLUDE_DIR SDL2_MIXER_LIBRARY SDL2_MIXER_EXTRA_DEPS
	MODPLUG_LIBRARY FLAC_LIBRARY OGG_LIBRARY VORBIS_LIBRARY)
