# SNDFILE_LIBRARIES
# SNDFILE_INCLUDE_DIR
# SndFile_FOUND

include(FindPackageHandleStandardArgs)

find_library(OGG_LIBRARY ogg)
find_library(FLAC_LIBRARY FLAC)
find_library(VORBIS_LIBRARY vorbis)
find_library(VORBISENC_LIBRARY vorbisenc)

find_library(SNDFILE_LIBRARY sndfile)
find_path(SNDFILE_INCLUDE_DIR sndfile.h)

if((EXISTS ${SNDFILE_LIBRARY}) AND (EXISTS ${SNDFILE_INCLUDE_DIR}))
  foreach(i OGG_LIBRARY FLAC_LIBRARY VORBIS_LIBRARY VORBISENC_LIBRARY)
    if(EXISTS ${${i}})
      list(APPEND SNDFILE_LIBRARIES "${${i}}")
    endif()
  endforeach()

  list(APPEND SNDFILE_LIBRARIES "${SNDFILE_LIBRARY}")
  message(STATUS "sndfile dependencies: ${SNDFILE_LIBRARIES}")
endif()

find_package_handle_standard_args(SndFile
	REQUIRED_VARS SNDFILE_INCLUDE_DIR SNDFILE_LIBRARIES)
