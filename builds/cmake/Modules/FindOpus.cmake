find_path(OPUS_INCLUDE_DIR_INTERNAL opus/opusfile.h)
find_library(OPUS_LIBRARY NAMES opusfile libopusfile)
if(EXISTS "${OPUS_INCLUDE_DIR_INTERNAL}")
  set(OPUS_INCLUDE_DIR "${OPUS_INCLUDE_DIR_INTERNAL}/opus")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG OPUS_INCLUDE_DIR OPUS_LIBRARY)

set(OPUS_INCLUDE_DIRS ${OPUS_INCLUDE_DIR})
set(OPUS_LIBRARIES ${OPUS_LIBRARY})

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARY)
