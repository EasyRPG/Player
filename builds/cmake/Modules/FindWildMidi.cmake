find_path(WILDMIDI_INCLUDE_DIR wildmidi_lib.h)
find_library(WILDMIDI_LIBRARY NAMES wildmidi libwildmidi WildMidi libWildMidi)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WildMidi DEFAULT_MSG WILDMIDI_INCLUDE_DIR WILDMIDI_LIBRARY)

set(WILDMIDI_INCLUDE_DIRS ${WILDMIDI_INCLUDE_DIR})
set(WILDMIDI_LIBRARIES ${WILDMIDI_LIBRARY})

mark_as_advanced(WILDMIDI_INCLUDE_DIR WILDMIDI_LIBRARY)
