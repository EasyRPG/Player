#.rst:
# FindOpusfile
# -----------
#
# Find the Opusfile Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Opusfile::Opusfile``
#   The ``Opusfile`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``OPUSFILE_INCLUDE_DIRS``
#   where to find Opusfile headers.
# ``OPUSFILE_LIBRARIES``
#   the libraries to link against to use Opusfile.
# ``OPUSFILE_FOUND``
#   true if the Opusfile headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_OPUSFILE QUIET opusfile)

# Look for the header file.
find_path(OPUSFILE_INCLUDE_DIR
	NAMES opusfile.h
	PATH_SUFFIXES opus
	HINTS ${PC_OPUSFILE_INCLUDE_DIRS})

# Look for the library.
# Allow OPUSFILE_LIBRARY to be set manually, as the location of the Opusfile library
if(NOT OPUSFILE_LIBRARY)
	find_library(OPUSFILE_LIBRARY
		NAMES libopusfile opusfile
		HINTS ${PC_OPUSFILE_LIBRARY_DIRS})
endif()

# Additional dependencies
find_library(OGG_LIBRARY
	NAMES libogg ogg)

find_library(OPUS_LIBRARY
	NAMES libopus opus)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opusfile
	REQUIRED_VARS OPUSFILE_LIBRARY OPUSFILE_INCLUDE_DIR)

if(OPUSFILE_FOUND)
	set(OPUSFILE_INCLUDE_DIRS ${OPUSFILE_INCLUDE_DIR})

	if(NOT OPUSFILE_LIBRARIES)
		set(OPUSFILE_LIBRARIES ${OPUSFILE_LIBRARIES})
	endif()

	if(NOT TARGET Opusfile::Opusfile)
		add_library(Opusfile::Opusfile UNKNOWN IMPORTED)
		set_target_properties(Opusfile::Opusfile PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${OPUSFILE_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${OGG_LIBRARY};${OPUS_LIBRARY}"
			IMPORTED_LOCATION "${OPUSFILE_LIBRARY}")
	endif()
endif()

mark_as_advanced(OPUSFILE_INCLUDE_DIR OPUSFILE_LIBRARY OPUS_LIBRARY)
