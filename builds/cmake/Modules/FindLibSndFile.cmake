#.rst:
# FindLibSndFile
# -----------
#
# Find the LibSndFile Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``LibSndFile::LibSndFile``
#   The ``LibSndFile`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``LIBSNDFILE_INCLUDE_DIRS``
#   where to find LibSndFile headers.
# ``LIBSNDFILE_LIBRARIES``
#   the libraries to link against to use LibSndFile.
# ``LIBSNDFILE_FOUND``
#   true if the LibSndFile headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_check_modules(PC_LIBSNDFILE QUIET sndfile)

# Look for the header file.
find_path(LIBSNDFILE_INCLUDE_DIR
	NAMES sndfile.h
	PATH_SUFFIXES libsndfile sndfile
	HINTS ${PC_LIBSNDFILE_INCLUDE_DIRS})

# Look for the library.
# Allow LIBSNDFILE_LIBRARY to be set manually, as the location of the LibSndFile library
if(NOT LIBSNDFILE_LIBRARY)
	find_library(LIBSNDFILE_LIBRARY
		NAMES libsndfile sndfile
		HINTS ${PC_LIBSNDFILE_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSndFile
	REQUIRED_VARS LIBSNDFILE_LIBRARY LIBSNDFILE_INCLUDE_DIR)

if(LIBSNDFILE_FOUND)
	set(LIBSNDFILE_INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIR})

	if(NOT LIBSNDFILE_LIBRARIES)
		set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBRARIES})
	endif()

	if(NOT TARGET LibSndFile::LibSndFile)
		add_library(LibSndFile::LibSndFile UNKNOWN IMPORTED)
		set_target_properties(LibSndFile::LibSndFile PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${LIBSNDFILE_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${LIBSNDFILE_LIBRARY}")
	endif()
endif()

mark_as_advanced(LIBSNDFILE_INCLUDE_DIR LIBSNDFILE_LIBRARY)
