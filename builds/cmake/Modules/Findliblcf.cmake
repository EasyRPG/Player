find_path(LIBLCF_INCLUDE_DIR liblcf/reader_lcf.h)
find_library(LIBLCF_LIBRARY NAMES lcf liblcf)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBLCF DEFAULT_MSG LIBLCF_INCLUDE_DIR LIBLCF_LIBRARY)

set(LIBLCF_INCLUDE_DIRS ${LIBLCF_INCLUDE_DIR})
set(LIBLCF_LIBRARIES ${LIBLCF_LIBRARY})

mark_as_advanced(LIBLCF_INCLUDE_DIR LIBLCF_LIBRARY)
