#.rst:
# FindVTune
# -----------
#
# Find the VTune ITT Library
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``VTune::ITT``
#   The Intel ``ITT`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``VTUNE_ROOT``
#   The root installation folder for VTune
# ``ITT_INCLUDE_DIRS``
#   where to find ITT headers.
# ``ITT_LIBRARIES``
#   the libraries to link against to use ITT.
# ``ITT_FOUND``
#   true if the ITT headers and libraries were found.

if (UNIX)
	set(VTUNE_DEFAULT_ROOT /opt/intel/vtune_amplifier)
endif()

if(NOT VTUNE_ARCH)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(VTUNE_ARCH "64")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(VTUNE_ARCH "32")
	else()
		message( FATAL_ERROR "Unable to determine 32 or 64 bit architecture for intel" )
	endif()
endif()

if(NOT VTUNE_ROOT)
	if(EXISTS "${VTUNE_DEFAULT_ROOT}")
		set(VTUNE_ROOT "${VTUNE_DEFAULT_ROOT}")
	endif()
endif()

if(VTUNE_ROOT)
	list(APPEND CMAKE_PREFIX_PATH "${VTUNE_ROOT}")
	list(APPEND CMAKE_LIBRARY_PATH "${VTUNE_ROOT}/lib${VTUNE_ARCH}")
endif()

find_package(PkgConfig QUIET)

pkg_check_modules(PC_ITT QUIET ittnotify)

# Look for the header file.
find_path(ITT_INCLUDE_DIR
	NAMES ittnotify.h
	HINTS ${PC_ITT_INCLUDE_DIRS})

# Look for the library.
# Allow ITT_LIBRARY to be set manually, as the location of the VTune ITT library
if(NOT ITT_LIBRARY)
	find_library(ITT_LIBRARY
		NAMES libittnotify ittnotify
		HINTS ${PC_ITT_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ITT
	REQUIRED_VARS ITT_LIBRARY ITT_INCLUDE_DIR)

if(ITT_FOUND)
	set(ITT_INCLUDE_DIRS ${ITT_INCLUDE_DIR})

	if(NOT ITT_LIBRARIES)
		set(ITT_LIBRARIES ${ITT_LIBRARIES})
	endif()

	if(NOT TARGET VTune::ITT)
		add_library(VTune::ITT STATIC IMPORTED)
		set_target_properties(VTune::ITT PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${ITT_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${ITT_LIBRARY}")
		if(UNIX)
			set_target_properties(VTune::ITT PROPERTIES INTERFACE_LINK_LIBRARIES "dl;pthread")
		endif()
	endif()
endif()

mark_as_advanced(ITT_INCLUDE_DIR ITT_LIBRARY)
