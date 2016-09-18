include(FindPackageHandleStandardArgs)

find_path(XMP_INCLUDE_DIR_INTERNAL xmp.h)
find_library(XMP_LIBRARY xmp libxmp xmp-lite libxmp-lite)
if(EXISTS "${XMP_INCLUDE_DIR_INTERNAL}")
  set(XMP_INCLUDE_DIR "${XMP_INCLUDE_DIR_INTERNAL}")
else()
  find_path(XMP_INCLUDE_DIR_INTERNAL libxmp/xmp.h)
  if(EXISTS "${XMP_INCLUDE_DIR_INTERNAL}")
    set(XMP_INCLUDE_DIR "${XMP_INCLUDE_DIR_INTERNAL}/libxmp")
  else()
    find_path(XMP_INCLUDE_DIR_INTERNAL libxmp-lite/xmp.h)
    if(EXISTS "${XMP_INCLUDE_DIR_INTERNAL}")
      set(XMP_INCLUDE_DIR "${XMP_INCLUDE_DIR_INTERNAL}/libxmp-lite")
    endif()
  endif()
endif()

find_package_handle_standard_args(XMP REQUIRED_VARS XMP_INCLUDE_DIR XMP_LIBRARY)

set(XMP_INCLUDE_DIRS ${XMP_INCLUDE_DIR})
set(XMP_LIBRARIES ${XMP_LIBRARY})

mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY)
