cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

project(EasyRPG_Player CXX C)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_definitions(-D _DEBUG=1)
endif()
	
add_definitions(
  ${PNG_DEFINITIONS}

  -D USE_SDL=1)
  
if(CMAKE_GENERATOR MATCHES "Makefile")
  add_definitions(-fno-rtti)
endif()

# Endianess check
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  include(TestBigEndian)
  test_big_endian(WORDS_BIGENDIAN)
  if(WORDS_BIGENDIAN)
    add_definitions(-D WORDS_BIGENDIAN=1)
  endif()
endif()

# Adds a library to the liblist if COND is true and the lib is found
# When REQ is true the lib is required
function(find_lib_int LIBNAME COND REQ)
	if(${COND})
		list(APPEND EASYRPG_LIB_LIST ${LIBNAME})
	
		if(${REQ})
			find_package(${LIBNAME} REQUIRED)
		else()
			find_package(${LIBNAME})
		endif()
		
		string(TOUPPER ${LIBNAME} LIBNAMEUP)
		include_directories(${${LIBNAMEUP}_INCLUDE_DIR} ${${LIBNAMEUP}_INCLUDE_DIRS})
		set(NEW_LIBS ${${LIBNAMEUP}_LIBRARY} ${${LIBNAMEUP}_LIBRARIES} PARENT_SCOPE)
	endif()

	set(RETVAL ${${LIBNAME}_FOUND} PARENT_SCOPE)
	# Export _FOUND variable to parent scope
	set(${LIBNAME}_FOUND ${${LIBNAME}_FOUND} PARENT_SCOPE)
endfunction()

function(find_lib_req LIBNAME COND)
	find_lib_int(${LIBNAME} ${COND} ON)
	
	# Export _FOUND variable to parent scope which is global scope
	set(${LIBNAME}_FOUND ${${LIBNAME}_FOUND} PARENT_SCOPE)
	set(EASYRPG_PLAYER_LIBRARIES ${EASYRPG_PLAYER_LIBRARIES} ${NEW_LIBS} PARENT_SCOPE)
	set(RETVAL ${RETVAL} PARENT_SCOPE)
endfunction()

function(find_lib LIBNAME COND)
	find_lib_int(${LIBNAME} ${COND} OFF)
	
	# Export _FOUND variable to parent scope which is global scope
	set(${LIBNAME}_FOUND ${${LIBNAME}_FOUND} PARENT_SCOPE)
	set(EASYRPG_PLAYER_LIBRARIES ${EASYRPG_PLAYER_LIBRARIES} ${NEW_LIBS} PARENT_SCOPE)
	set(RETVAL ${RETVAL} PARENT_SCOPE)
endfunction()

# liblcf
option(PLAYER_BUILD_LIBLCF "Instead of detecting liblcf the liblcf repository is cloned into lib/liblcf and built together with the Player. This is convenient for development")
set(PLAYER_BUILD_LIBLCF_GIT "https://github.com/EasyRPG/liblcf.git" CACHE STRING "Git repository of liblcf to clone when building liblcf. Requires PLAYER_BUILD_LIBLCF=ON.")

if (PLAYER_BUILD_LIBLCF)
	# liblcf is built as part of this cmake file
	set(LIBLCF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf")
	find_package(Git REQUIRED)
	if(NOT EXISTS ${LIBLCF_PATH})
	execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1"
		"${PLAYER_BUILD_LIBLCF_GIT}"
		"${LIBLCF_PATH}")
	endif()
	list(APPEND CMAKE_MODULE_PATH
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules")
	add_subdirectory(
		${LIBLCF_PATH}/builds/cmake
		${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/liblcf)
	link_directories(${ICU_LIBRARY_DIRS})
else()
	# Use system package
	find_lib_req(liblcf ON)
endif()

# Detect all required libraries
foreach(i Pixman ZLIB PNG SDL2)
	find_lib_req(${i} ON)
endforeach()

# Always enable Wine registry support on non-Windows
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
else()
	add_definitions(-DHAVE_WINE=1)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
find_lib("Freetype" PLAYER_WITH_FREETYPE)
if(${RETVAL})
	add_definitions(-DHAVE_FREETYPE)
	option(PLAYER_WITH_HARFBUZZ "Enable HarfBuzz text shaping (Requires FreeType)" ON)
	find_lib("Harfbuzz" PLAYER_WITH_HARFBUZZ)
	if(${RETVAL})
		add_definitions(-DHAVE_HARFBUZZ)
	endif()
endif()

# Sound system to use
set(PLAYER_AUDIO_BACKEND "SDL2_mixer" CACHE STRING "Audio system to use. The SDL2_mixer audio system provides advanced features required by RPG Maker. Options: SDL2_mixer OpenAL OFF")
set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL2 OpenAL)

# Configure Audio backends

if(${PLAYER_AUDIO_BACKEND} MATCHES "SDL2_mixer")
	# SDL2_mixer Audio
	find_lib_req(SDL2_mixer ON)
	add_definitions(-D HAVE_SDL_MIXER=1)
	
	# speexdsp
	option(PLAYER_WITH_SPEEXDSP "Resample audio with speexdsp." ON)	
	find_lib("speexdsp" PLAYER_WITH_SPEEXDSP)
	if(${RETVAL})
		add_definitions(-DHAVE_LIBSPEEXDSP)
		add_definitions(-DUSE_AUDIO_RESAMPLER)
	endif()
	
	# mpg123
	option(PLAYER_WITH_MPG123 "Play MP3 audio with libmpg123." ON)
	find_lib("mpg123" PLAYER_WITH_MPG123)
	if(${RETVAL})
		add_definitions(-DHAVE_MPG123)
	endif()

	# libsndfile
	option(PLAYER_WITH_LIBSNDFILE "Play WAV audio with libsndfile." ON)	
	find_lib("SndFile" PLAYER_WITH_LIBSNDFILE)
	if(${RETVAL})
		add_definitions(-DHAVE_LIBSNDFILE)
	endif()
	
	# libogg
	option(PLAYER_WITH_LIBOGG "Play OGG audio with libogg or tremor." ON)	
	find_lib("OggVorbis" PLAYER_WITH_SPEEXDSP)
	if(${RETVAL})
		add_definitions(-DHAVE_OGGVORBIS)
	endif()
	
	# wildmidi (optional, OFF)
	option(PLAYER_WITH_WILDMIDI "Play MIDI audio with wildmidi. (optional)" OFF)
	find_lib("wildmidi" PLAYER_WITH_WILDMIDI)
	if(${RETVAL})
		add_definitions(-DHAVE_WILDMIDI)
	endif()
	
	# Provide fmmidi options
	set(PLAYER_ENABLE_FMMIDI "Fallback" CACHE STRING "Enable internal MIDI sequencer. Fallback (default) mode will use it when the external MIDI library fails. Options: Fallback ON OFF")
	set_property(CACHE PLAYER_ENABLE_FMMIDI PROPERTY STRINGS Fallback ON OFF)
	if(${PLAYER_ENABLE_FMMIDI} MATCHES "Fallback")
		add_definitions(-DWANT_FMMIDI=2)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		add_definitions(-DWAND_FMMIDI=1)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		# noop
	else()
		message(FATAL_ERROR "Bad FmMidi setting")
	endif()	
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OpenAL")
	# OpenAL Audio
	add_definitions(-DNO_SDL_MIXER)
	find_package(OpenAL REQUIRED)
	find_package(SndFile REQUIRED)
	find_package(FluidSynth REQUIRED)
	include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR} ${FLUIDSYNTH_INCLUDE_DIR})
	list(APPEND EASYRPG_PLAYER_LIBRARIES ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES} ${FLUIDSYNTH_LIBRARY})
	add_definitions(-D HAVE_OPENAL=1)
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OFF")
	add_definitions(-DNO_SDL_MIXER)
elseif()
	message(FATAL_ERROR "Invalid Audio Backend")
endif()

if(PLAYER_BUILD_LIBLCF)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src/generated")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# library
set(EASYRPG_PLAYER_LIBRARIES_ALL ${EASYRPG_PLAYER_LIBRARIES} "${PROJECT_NAME}_Static")
if(PLAYER_BUILD_LIBLCF)
	list(APPEND EASYRPG_PLAYER_LIBRARIES liblcf)
endif()

# entry point
set(MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# static library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
list(REMOVE_ITEM SRCS ${MAIN_FILE})
list(APPEND SRCS ${SHINONOME_SRCS})
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	list(APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/resources/player.rc")
endif()
add_library(${PROJECT_NAME}_Static STATIC ${SRCS})

# executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${MAIN_FILE})
add_executable(${PROJECT_NAME}_com ${MAIN_FILE})
foreach(i ${PROJECT_NAME} ${PROJECT_NAME}_com)
	target_link_libraries(${i} ${EASYRPG_PLAYER_LIBRARIES_ALL})
	add_dependencies(${i} ${PROJECT_NAME}_Static)
	if(PLAYER_BUILD_LIBLCF)
		add_dependencies(${i} liblcf)
	endif()
endforeach()
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} DESTINATION bin)

# manpage
# Handled as extra target "man"

set(MAN_NAME easyrpg-player.6)
find_program(A2X_EXECUTABLE NAMES a2x a2x.py)
if(NOT A2X_EXECUTABLE STREQUAL "A2X_EXECUTABLE-NOTFOUND")
	add_custom_command(OUTPUT resources/${MAN_NAME}
		COMMAND ${A2X_EXECUTABLE} -a player_version="0.4.1" -f manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME}.adoc
		DEPENDS resources/${MAN_NAME}.adoc
		COMMENT "(Re-)building manpage ${MAN_NAME}"
		VERBATIM)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		add_custom_target(man ALL DEPENDS resources/${MAN_NAME})
		install(FILES resources/${MAN_NAME} DESTINATION share/man/man6)
	else()
		add_custom_target(man DEPENDS resources/${MAN_NAME})
	endif()
else()
	# no a2x, distribution archive?
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME})
		install(FILES resources/${MAN_NAME} DESTINATION share/man/man6)
	endif()
endif()

# CPack
set(CPACK_GENERATOR "ZIP" "TGZ")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	list(APPEND CPACK_GENERATOR "NSIS")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "0.4.1")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/player.ico")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

include(CPack)

# Doxygen
# Handled as extra target "doc"

find_package(Doxygen)
if(DOXYGEN_FOUND)
	# fake autotools variables
	set(DX_DOCDIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
	set(PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
	set(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
	configure_file(resources/Doxyfile.in resources/Doxyfile @ONLY)
	unset(DX_DOCDIR)
	unset(PACKAGE_VERSION)
	unset(srcdir)

	add_custom_target(player_doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/resources/Doxyfile
		DEPENDS ${SRCS} ${MAIN_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	add_custom_target(doc)
	add_dependencies(doc player_doc liblcf_doc)
endif()

# test
option(PLAYER_ENABLE_TESTS "Execute unit tests after compilation finishes" ON)

if (PLAYER_ENABLE_TESTS)
	enable_testing()

	set(TEST_GAME_REPOSITORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/TestGame")
	if(NOT EXISTS ${TEST_GAME_REPOSITORY_PATH})
		execute_process(COMMAND ${GIT_EXECUTABLE} clone
			"https://github.com/EasyRPG/TestGame.git"
			"${TEST_GAME_REPOSITORY_PATH}")
	endif()
	set(TEST_GAME_PATH "${TEST_GAME_REPOSITORY_PATH}/TestGame-2000")

	set(TEST_ENVS "RPG_TEST_GAME_PATH=${TEST_GAME_PATH}")
	if(EXISTS $ENV{RPG_RTP_PATH})
		get_filename_component(RPG_RTP_PATH "$ENV{RPG_RTP_PATH}" ABSOLUTE)
		message(STATUS "using rtp path ${RPG_RTP_PATH}")
		list(APPEND TEST_ENVS "RPG_RTP_PATH=${RPG_RTP_PATH}")
	endif()

	file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
	foreach(i ${TEST_FILES})
		get_filename_component(name "${i}" NAME_WE)
		add_executable(test_${name} ${i})
		target_link_libraries(test_${name} ${EASYRPG_PLAYER_LIBRARIES_ALL})
		add_dependencies(test_${name} ${PROJECT_NAME}_Static)

		add_test(
			NAME test_${name} WORKING_DIRECTORY ${TEST_GAME_PATH}
			COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_${name})
		set_tests_properties(test_${name}
			PROPERTIES ENVIRONMENT "${TEST_ENVS}")
	endforeach()
endif()

# Print summary
message(STATUS "")
message(STATUS "Summary:")
message(STATUS "----------")
if(PLAYER_BUILD_LIBLCF)
	message(STATUS "liblcf: Built from ${PLAYER_BUILD_LIBLCF_GIT}")
	message(STATUS "")
endif()
message(STATUS "Audio backend: ${PLAYER_AUDIO_BACKEND}")
if(${PLAYER_AUDIO_BACKEND} MATCHES "SDL2_mixer")
	message(STATUS "")

	if(wildmidi_FOUND)
		message(STATUS "MIDI playback: WildMidi")
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		message(STATUS "MIDI playback: built-in (FmMidi)")
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "Fallback")
		message(STATUS "MIDI playback: SDL2_mixer. Fallback to built-in (FmMidi)")
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		message(STATUS "MIDI playback: SDL2_mixer.")
	endif()
	
	if(mpg123_FOUND)
		message(STATUS "MP3 playback:  mpg123")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "MP3 playback:  SDL2_mixer. Some MP3 files will crash or play noise!")
	endif()
	
	if(OggVorbis_FOUND)
		message(STATUS "OGG playback:  libogg")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "OGG playback:  SDL2_mixer")
	endif()
	
	if(SndFile_FOUND)
		message(STATUS "WAV playback:  libsndfile")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "WAV playback:  SDL2_mixer. Some WAV formats are unsupported.")
	endif()
	
	if(speexdsp_FOUND)
		message(STATUS "Resampler:     speexdsp")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "Resampler:     SDL2. Resampling will not work correctly!")
	endif()
	
	if(SDL_MIXER_USED)
		message(STATUS "")
		message(STATUS " WARNING:")
		message(STATUS " Using SDL2_mixer directly for playback or resampling is not recommended.")
		message(STATUS " Some RPG Maker features (e.g. pitch changes) will not work.")
	endif()
endif()

message(STATUS "")

if(Harfbuzz_FOUND)
	message(STATUS "Font rendering: Freetype with Harfbuzz / built-in (Shinonome)")
elseif(FreeType_FOUND)
	message(STATUS "Font rendering: Freetype and built-in (Shinonome)")
else()
	message(STATUS "Font rendering: built-in (Shinonome)")
endif()

message(STATUS "----------")
message(STATUS "")

