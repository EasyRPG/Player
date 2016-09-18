# PIXMAN_INCLUDE_DIR - pixman include directory
# Pixman_FOUND - wether pixman is found
# PIXMAN_LIBRARY - pixman library

include(FindPackageHandleStandardArgs)

find_path(PIXMAN_INCLUDE_DIR_INTERNAL pixman-1/pixman.h)
find_library(PIXMAN_LIBRARY pixman-1)
if(EXISTS "${PIXMAN_INCLUDE_DIR_INTERNAL}")
  set(PIXMAN_INCLUDE_DIR "${PIXMAN_INCLUDE_DIR_INTERNAL}/pixman-1")
endif()

find_package_handle_standard_args(Pixman
	REQUIRED_VARS PIXMAN_INCLUDE_DIR PIXMAN_LIBRARY)

set(PIXMAN_INCLUDE_DIRS ${PIXMAN_INCLUDE_DIR})
set(PIXMAN_LIBRARIES ${PIXMAN_LIBRARY})

mark_as_advanced(PIXMAN_INCLUDE_DIR PIXMAN_LIBRARY)
