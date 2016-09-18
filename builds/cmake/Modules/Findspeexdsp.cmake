find_path(SPEEXDSP_INCLUDE_DIR speex/speex_resampler.h)
find_library(SPEEXDSP_LIBRARY NAMES speexdsp libspeexdsp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(speexdsp DEFAULT_MSG SPEEXDSP_INCLUDE_DIR SPEEXDSP_LIBRARY)

set(SPEEXDSP_INCLUDE_DIRS ${SPEEXDSP_INCLUDE_DIR})
set(SPEEXDSP_LIBRARIES ${SPEEXDSP_LIBRARY})

mark_as_advanced(SPEEXDSP_INCLUDE_DIR SPEEXDSP_LIBRARY)
