#.rst:
# FindVorbis
# -----------
#
# Find the Vorbisfile Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Vorbis::vorbisfile``
#   The ``Vorbisfile`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``VORBISFILE_INCLUDE_DIRS``
#   where to find Vorbisfile headers.
# ``VORBISFILE_LIBRARIES``
#   the libraries to link against to use Vorbisfile.
# ``VORBISFILE_FOUND``
#   true if the Vorbisfile headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_VORBISFILE QUIET vorbisfile)

# Look for the header file.
find_path(VORBISFILE_INCLUDE_DIR
	NAMES vorbisfile.h
	PATH_SUFFIXES vorbis
	HINTS ${PC_VORBISFILE_INCLUDE_DIRS})

# Look for the library.
# Allow VORBISFILE_LIBRARY to be set manually, as the location of the Vorbisfile library
if(NOT VORBISFILE_LIBRARY)
	find_library(VORBISFILE_LIBRARY
		NAMES libvorbisfile vorbisfile
		HINTS ${PC_VORBISFILE_LIBRARY_DIRS})
endif()

# Find additional dependencies
find_library(VORBIS_LIBRARY
	NAMES libvorbis vorbis
	HINTS ${PC_VORBISFILE_LIBRARY_DIRS})

find_library(OGG_LIBRARY
	NAMES libogg ogg
	HINTS ${PC_VORBISFILE_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vorbis
	REQUIRED_VARS VORBISFILE_LIBRARY VORBISFILE_INCLUDE_DIR)

if(VORBIS_FOUND)
	set(VORBISFILE_INCLUDE_DIRS ${VORBISFILE_INCLUDE_DIR})

	if(NOT VORBISFILE_LIBRARIES)
		set(VORBISFILE_LIBRARIES ${VORBISFILE_LIBRARIES})
	endif()

	if(NOT TARGET Vorbis::vorbisfile)
		add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
		set_target_properties(Vorbis::vorbisfile PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${VORBISFILE_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${VORBIS_LIBRARY};${OGG_LIBRARY}"
			IMPORTED_LOCATION "${VORBISFILE_LIBRARY}")
	endif()
endif()

mark_as_advanced(VORBISFILE_INCLUDE_DIR VORBISFILE_LIBRARY OGG_LIBRARY VORBIS_LIBRARY)
