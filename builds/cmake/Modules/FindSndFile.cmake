# SNDFILE_LIBRARIES
# SNDFILE_INCLUDE_DIR
# SndFile_FOUND

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

  message(STATUS "Found sndfile: ${SNDFILE_LIBRARIES}")
else()
  set(SndFile_FOUND FALSE)
  if(SndFile_FIND_REQUIRED)
    message(SEND_ERROR "sndfile not found")
  else()
    message(STATUS "sndfile not found")
  endif()
endif()
