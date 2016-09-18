find_path(SNDFILE_INCLUDE_DIR sndfile.h)
find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SndFile DEFAULT_MSG SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)

set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})
set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
