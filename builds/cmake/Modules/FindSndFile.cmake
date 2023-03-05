#.rst:
# FindSndFile
# -----------
#
# Find the SndFile Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``SndFile::sndfile``
#   The ``SndFile`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``SNDFILE_INCLUDE_DIRS``
#   where to find SndFile headers.
# ``SNDFILE_LIBRARIES``
#   the libraries to link against to use SndFile.
# ``SNDFILE_FOUND``
#   true if the SndFile headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_SNDFILE QUIET sndfile)

# Look for the header file.
find_path(SNDFILE_INCLUDE_DIR
	NAMES sndfile.h
	PATH_SUFFIXES libsndfile sndfile
	HINTS ${PC_SNDFILE_INCLUDE_DIRS})

# Look for the library.
# Allow SNDFILE_LIBRARY to be set manually, as the location of the LibSndFile library
if(NOT SNDFILE_LIBRARY)
	find_library(SNDFILE_LIBRARY
		NAMES libsndfile sndfile
		HINTS ${PC_SNDFILE_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SndFile
	REQUIRED_VARS SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)

if(SNDFILE_FOUND)
	set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})

	if(NOT SNDFILE_LIBRARIES)
		set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARIES})
	endif()

	if(NOT TARGET SndFile::sndfile)
		add_library(SndFile::sndfile UNKNOWN IMPORTED)
		set_target_properties(SndFile::sndfile PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SNDFILE_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${SNDFILE_LIBRARY}")
	endif()
endif()

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
