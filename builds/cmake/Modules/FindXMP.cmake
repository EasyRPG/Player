#.rst:
# FindXMP
# -----------
#
# Find the XMP Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``XMP::XMP``
#   The ``XMP`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``XMP_INCLUDE_DIRS``
#   where to find XMP headers.
# ``XMP_LIBRARIES``
#   the libraries to link against to use XMP.
# ``XMP_FOUND``
#   true if the XMP headers and libraries were found.

find_package(PkgConfig QUIET)

pkg_search_module(PC_XMP QUIET libxmp libxmp-lite)

# Look for the header file.
find_path(XMP_INCLUDE_DIR
	NAMES xmp.h
	PATH_SUFFIXES libxmp xmp libxmp-lite xmp-lite
	HINTS ${PC_XMP_INCLUDE_DIRS})

# Look for the library.
# Allow XMP_LIBRARY to be set manually, as the location of the XMP library
if(NOT XMP_LIBRARY)
	find_library(XMP_LIBRARY
		NAMES libxmp xmp libxmp-static xmp-static libxmp-lite xmp-lite
		HINTS ${PC_XMP_LIBRARY_DIRS})
endif()


if(XMP_INCLUDE_DIR AND EXISTS "${XMP_INCLUDE_DIR}/xmp.h")
	file(STRINGS "${XMP_INCLUDE_DIR}/xmp.h" XMP_VERSION_LINE REGEX "^#define XMP_VERSION \"[0-9.]+\"$")
	string(REGEX REPLACE "^#define XMP_VERSION \"([0-9.]+)\"$" "\\1" XMP_VERSION "${XMP_VERSION_LINE}")
	unset(XMP_VERSION_LINE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XMP
	REQUIRED_VARS XMP_LIBRARY XMP_INCLUDE_DIR
	VERSION_VAR XMP_VERSION)

if(XMP_FOUND)
	set(XMP_INCLUDE_DIRS ${XMP_INCLUDE_DIR})

	if(NOT XMP_LIBRARIES)
		set(XMP_LIBRARIES ${XMP_LIBRARIES})
	endif()

	if(NOT TARGET XMP::XMP)
		add_library(XMP::XMP UNKNOWN IMPORTED)
		set_target_properties(XMP::XMP PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${XMP_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${XMP_LIBRARY}")
		if(WIN32)
			set_target_properties(XMP::XMP PROPERTIES
				INTERFACE_COMPILE_DEFINITIONS "LIBXMP_STATIC=1")
		endif()
	endif()
endif()

mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY)
