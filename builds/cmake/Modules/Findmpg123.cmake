find_path(MPG123_INCLUDE_DIR mpg123.h)
find_library(MPG123_LIBRARY NAMES mpg123 libmpg123)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpg123 DEFAULT_MSG MPG123_INCLUDE_DIR MPG123_LIBRARY)

set(MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIR})
set(MPG123_LIBRARIES ${MPG123_LIBRARY})

mark_as_advanced(MPG123_INCLUDE_DIR MPG123_LIBRARY)
