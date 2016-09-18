find_path(OGGVORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
find_library(OGGVORBIS_LIBRARY NAMES vorbisfile libvorbisfile)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OggVorbis DEFAULT_MSG OGGVORBIS_INCLUDE_DIR OGGVORBIS_LIBRARY)

set(OGGVORBIS_INCLUDE_DIRS ${OGGVORBIS_INCLUDE_DIR})
set(OGGVORBIS_LIBRARIES ${OGGVORBIS_LIBRARY})

mark_as_advanced(OGGVORBIS_INCLUDE_DIR OGGVORBIS_LIBRARY)
