# https://github.com/scivision/cmake-patch-file/blob/main/cmake/PatchFile.cmake
if(WIN32)
	get_filename_component(GIT_DIR ${GIT_EXECUTABLE} DIRECTORY)
	get_filename_component(GIT_DIR ${GIT_DIR} DIRECTORY)
endif()
find_program(PATCH_EXECUTABLE NAMES patch HINTS ${GIT_DIR} PATH_SUFFIXES usr/bin)
