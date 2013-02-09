# PIXMAN_INCLUDE_DIR - pixman include directory
# Pixman_FOUND - wether pixman is found
# PIXMAN_LIBRARY - pixman library

find_path(PIXMAN_INCLUDE_DIR_INTERNAL pixman-1/pixman.h)
find_library(PIXMAN_LIBRARY pixman-1)
if((EXISTS ${PIXMAN_INCLUDE_DIR_INTERNAL}) AND (EXISTS ${PIXMAN_LIBRARY}))
  message(STATUS "Found Pixman: ${PIXMAN_LIBRARY}")
  set(Pixman_FOUND TRUE)
  set(PIXMAN_INCLUDE_DIR "${PIXMAN_INCLUDE_DIR_INTERNAL}/pixman-1")
else()
  set(Pixman_FOUND FALSE)
  if(Pixman_FOUND_REQUIRED)
    message(SEND_ERROR "Pixman not found")
  else()
    message(STATUS "Pixman not found")
  endif()
endif()
