cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
include(CMakeDependentOption)

project(EasyRPG_Player CXX C)
set(PACKAGE_VERSION "0.5.2")

# Source Files
set(PLAYER_SRCS
	src/async_handler.cpp
	src/audio_al.cpp
	src/audio.cpp
	src/audio_decoder.cpp
	src/audio_generic.cpp
	src/audio_resampler.cpp
	src/audio_sdl_mixer.cpp
	src/audio_sdl.cpp
	src/audio_secache.cpp
	src/background.cpp
	src/baseui.cpp
	src/battle_animation.cpp
	src/bitmap.cpp
	src/cache.cpp
	src/color.cpp
	src/decoder_fmmidi.cpp
	src/decoder_libsndfile.cpp
	src/decoder_mpg123.cpp
	src/decoder_oggvorbis.cpp
	src/decoder_opus.cpp
	src/decoder_wav.cpp
	src/decoder_wildmidi.cpp
	src/decoder_xmp.cpp
	src/filefinder.cpp
	src/font.cpp
	src/fps_overlay.cpp
	src/frame.cpp
	src/game_actor.cpp
	src/game_actors.cpp
	src/game_battlealgorithm.cpp
	src/game_battle.cpp
	src/game_battler.cpp
	src/game_character.cpp
	src/game_commonevent.cpp
	src/game_enemy.cpp
	src/game_enemyparty.cpp
	src/game_event.cpp
	src/game_interpreter_battle.cpp
	src/game_interpreter.cpp
	src/game_interpreter_map.cpp
	src/game_map.cpp
	src/game_message.cpp
	src/game_party_base.cpp
	src/game_party.cpp
	src/game_picture.cpp
	src/game_player.cpp
	src/game_screen.cpp
	src/game_switches.cpp
	src/game_system.cpp
	src/game_targets.cpp
	src/game_temp.cpp
	src/game_variables.cpp
	src/game_vehicle.cpp
	src/graphics.cpp
	src/hslrgb.cpp
	src/image_bmp.cpp
	src/image_png.cpp
	src/image_xyz.cpp
	src/input_buttons_desktop.cpp
	src/input_buttons_gekko.cpp
	src/input_buttons_opendingux.cpp
	src/input_buttons_psp.cpp
	src/input_source.cpp
	src/input.cpp
	src/main_data.cpp
	src/message_overlay.cpp
	src/output.cpp
	src/plane.cpp
	src/player.cpp
	src/rect.cpp
	src/registry.cpp
	src/registry_wine.cpp
	src/rtp_table.cpp
	src/scene_actortarget.cpp
	src/scene_battle.cpp
	src/scene_battle_rpg2k3.cpp
	src/scene_battle_rpg2k.cpp
	src/scene.cpp
	src/scene_debug.cpp
	src/scene_end.cpp
	src/scene_equip.cpp
	src/scene_file.cpp
	src/scene_gamebrowser.cpp
	src/scene_gameover.cpp
	src/scene_item.cpp
	src/scene_load.cpp
	src/scene_logo.cpp
	src/scene_map.cpp
	src/scene_menu.cpp
	src/scene_name.cpp
	src/scene_order.cpp
	src/scene_save.cpp
	src/scene_shop.cpp
	src/scene_skill.cpp
	src/scene_status.cpp
	src/scene_teleport.cpp
	src/scene_title.cpp
	src/screen.cpp
	src/sdl_ui.cpp
	src/shinonome_gothic.cpp
	src/shinonome_mincho.cpp
	src/bitmapfont_wqy.cpp
	src/bitmapfont_ttyp0.cpp
	src/bitmapfont_rmg2000.cpp
	src/sprite_airshipshadow.cpp
	src/sprite_battler.cpp
	src/sprite_character.cpp
	src/sprite.cpp
	src/spriteset_battle.cpp
	src/spriteset_map.cpp
	src/sprite_timer.cpp
	src/text.cpp
	src/tilemap.cpp
	src/tilemap_layer.cpp
	src/tone.cpp
	src/utils.cpp
	src/util_win.cpp
	src/weather.cpp
	src/window_about.cpp
	src/window_actorinfo.cpp
	src/window_actorstatus.cpp
	src/window_actortarget.cpp
	src/window_base.cpp
	src/window_battlecommand.cpp
	src/window_battlemessage.cpp
	src/window_battleoption.cpp
	src/window_battlestatus.cpp
	src/window_command.cpp
	src/window.cpp
	src/window_equip.cpp
	src/window_equipitem.cpp
	src/window_equipstatus.cpp
	src/window_face.cpp
	src/window_gamelist.cpp
	src/window_gold.cpp
	src/window_help.cpp
	src/window_item.cpp
	src/window_keyboard.cpp
	src/window_menustatus.cpp
	src/window_message.cpp
	src/window_name.cpp
	src/window_numberinput.cpp
	src/window_savefile.cpp
	src/window_selectable.cpp
	src/window_shopbuy.cpp
	src/window_shop.cpp
	src/window_shopnumber.cpp
	src/window_shopparty.cpp
	src/window_shopsell.cpp
	src/window_shopstatus.cpp
	src/window_skill.cpp
	src/window_skillstatus.cpp
	src/window_targetstatus.cpp
	src/window_teleport.cpp
	src/window_varlist.cpp
)

# Include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# Extra FindModule files
list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")

# Add compile flags
if(CMAKE_GENERATOR MATCHES "Makefile")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_definitions(-D_DEBUG)
endif()

add_definitions(-DUSE_SDL)

# Endianess check
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	include(TestBigEndian)
	test_big_endian(WORDS_BIGENDIAN)
	if(WORDS_BIGENDIAN)
		add_definitions(-DWORDS_BIGENDIAN)
	endif()
endif()

# Adds a library to the liblist if COND is true and the lib is found
# When COND is REQUIRED the lib is required
function(find_lib LIBNAME COND)
	set(REQ OFF)
	if(${COND} MATCHES "REQUIRED")
		set(COND ON)
		set(REQ ON)
	endif()

	if(${COND})
		if(${REQ})
			find_package(${LIBNAME} REQUIRED)
		else()
			find_package(${LIBNAME})
		endif()
		
		if(${${LIBNAME}_FOUND})
			string(TOUPPER ${LIBNAME} LIBNAMEUP)
			include_directories(${${LIBNAMEUP}_INCLUDE_DIRS})
			set(EASYRPG_PLAYER_LIBRARIES ${EASYRPG_PLAYER_LIBRARIES} ${${LIBNAMEUP}_LIBRARIES} PARENT_SCOPE)
		endif()

		set(${LIBNAME}_FOUND ${${LIBNAME}_FOUND} PARENT_SCOPE)
	endif()
endfunction()

# liblcf
option(PLAYER_BUILD_LIBLCF "Instead of detecting liblcf the liblcf repository is cloned into lib/liblcf and built together with the Player. This is convenient for development" OFF)
set(PLAYER_BUILD_LIBLCF_GIT "https://github.com/EasyRPG/liblcf.git" CACHE STRING "Git repository of liblcf to clone when building liblcf. Requires PLAYER_BUILD_LIBLCF=ON.")

if(PLAYER_BUILD_LIBLCF)
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
else()
	# Use system package
	find_lib(liblcf REQUIRED)
endif()

# Detect all required libraries
foreach(i Pixman ZLIB PNG SDL2)
	find_lib(${i} REQUIRED)
endforeach()

# Always enable Wine registry support on non-Windows
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DHAVE_WINE)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_HARFBUZZ "Enable HarfBuzz text shaping (Requires FreeType)" ON "PLAYER_WITH_FREETYPE" OFF)
find_lib("Freetype" PLAYER_WITH_FREETYPE)
if(Freetype_FOUND)
	add_definitions(-DHAVE_FREETYPE)
	find_lib("Harfbuzz" PLAYER_WITH_HARFBUZZ)
	if(Harfbuzz_FOUND)
		add_definitions(-DHAVE_HARFBUZZ)
	endif()
endif()

# Sound system to use
set(PLAYER_AUDIO_BACKEND "Auto" CACHE STRING "Audio system to use. Options: Auto SDL2_mixer SDL2 OpenAL OFF")
set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS Auto SDL2 SDL2_mixer OpenAL OFF)

# Configure Audio backends
if(${PLAYER_AUDIO_BACKEND} MATCHES "Auto")
	option(PLAYER_WITH_SDL2_MIXER "Use SDL2 mixer when available." ON)
	find_lib("SDL2_mixer" PLAYER_WITH_SDL2_MIXER)
	if(SDL2_mixer_FOUND)
		set(PLAYER_AUDIO_BACKEND "SDL2_mixer")
	else()
		set(PLAYER_AUDIO_BACKEND "SDL2")
	endif()
endif()

if(${PLAYER_AUDIO_BACKEND} MATCHES "^SDL2$")
	add_definitions(-DSUPPORT_AUDIO)

	# wildmidi (on for non-mixer)
	option(PLAYER_WITH_WILDMIDI "Play MIDI audio with wildmidi." ON)
	find_lib("WildMidi" PLAYER_WITH_WILDMIDI)
	if(WildMidi_FOUND)
		add_definitions(-DHAVE_WILDMIDI)
	endif()
	
	# Provide fmmidi options
	option(PLAYER_ENABLE_FMMIDI "Enable internal MIDI sequencer. Will be used when external MIDI library fails." ON)
	if(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		add_definitions(-DWANT_FMMIDI)
	endif()	
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "SDL2_mixer")
	add_definitions(-DSUPPORT_AUDIO)

	# SDL2_mixer Audio
	find_lib(SDL2_mixer REQUIRED)
	add_definitions(-DHAVE_SDL_MIXER)
	
	# wildmidi (optional, OFF)
	option(PLAYER_WITH_WILDMIDI "Play MIDI audio with wildmidi. (optional)" OFF)
	find_lib("WildMidi" PLAYER_WITH_WILDMIDI)
	if(WildMidi_FOUND)
		add_definitions(-DHAVE_WILDMIDI)
	endif()
	
	# Provide fmmidi options
	set(PLAYER_ENABLE_FMMIDI "Fallback" CACHE STRING "Enable internal MIDI sequencer. Fallback (default) mode will use it when the external MIDI library fails. Options: Fallback ON OFF")
	set_property(CACHE PLAYER_ENABLE_FMMIDI PROPERTY STRINGS Fallback ON OFF)
	if(${PLAYER_ENABLE_FMMIDI} MATCHES "Fallback")
		add_definitions(-DWANT_FMMIDI=2)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		add_definitions(-DWANT_FMMIDI=1)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		# noop
	else()
		message(FATAL_ERROR "Bad FmMidi setting")
	endif()
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OpenAL")
	# OpenAL Audio
	add_definitions(-DSUPPORT_AUDIO)
	
	find_package(OpenAL REQUIRED)
	find_package(SndFile REQUIRED)
	find_package(FluidSynth REQUIRED)
	include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR} ${FLUIDSYNTH_INCLUDE_DIR})
	list(APPEND EASYRPG_PLAYER_LIBRARIES ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES} ${FLUIDSYNTH_LIBRARY})
	add_definitions(-DHAVE_OPENAL)
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OFF")

else()
	message(FATAL_ERROR "Invalid Audio Backend")
endif()

if(${PLAYER_AUDIO_BACKEND} MATCHES "^SDL.*$")
	# speexdsp
	option(PLAYER_WITH_SPEEXDSP "Resample audio with speexdsp." ON)	
	find_lib("speexdsp" PLAYER_WITH_SPEEXDSP)
	if(speexdsp_FOUND)
		add_definitions(-DHAVE_LIBSPEEXDSP)
	endif()
	
	# mpg123
	option(PLAYER_WITH_MPG123 "Play MP3 audio with libmpg123." ON)
	find_lib("mpg123" PLAYER_WITH_MPG123)
	if(mpg123_FOUND)
		add_definitions(-DHAVE_MPG123)
	endif()

	# libsndfile
	option(PLAYER_WITH_LIBSNDFILE "Play WAV audio with libsndfile." ON)	
	find_lib("SndFile" PLAYER_WITH_LIBSNDFILE)
	if(SndFile_FOUND)
		add_definitions(-DHAVE_LIBSNDFILE)
	endif()
	
	# libvorbis
	option(PLAYER_WITH_OGGVORBIS "Play Ogg Vorbis audio with libvorbis." ON)
	find_lib("OggVorbis" PLAYER_WITH_OGGVORBIS)
	if(OggVorbis_FOUND)
		add_definitions(-DHAVE_OGGVORBIS)
	endif()

	# opusfile
	option(PLAYER_WITH_OPUS "Play Opus audio with opusfile." ON)
	find_lib("Opus" PLAYER_WITH_OPUS)
	if(Opus_FOUND)
		add_definitions(-DHAVE_OPUS)
	endif()

	# xmp (lite)
	option(PLAYER_WITH_XMP "Play MOD audio with libxmp." ON)
	find_lib("XMP" PLAYER_WITH_XMP)
	if(XMP_FOUND)
		add_definitions(-DHAVE_XMP)
	endif()
	
	if(NOT ${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		list(APPEND PLAYER_SRCS
			src/midisequencer.cpp
			src/midisynth.cpp
		)
	endif()	
endif()

# Additional include directories
if(PLAYER_BUILD_LIBLCF)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/src/generated")
endif()

# Libraries
set(EASYRPG_PLAYER_LIBRARIES_ALL ${EASYRPG_PLAYER_LIBRARIES} "${PROJECT_NAME}_Static")
if(PLAYER_BUILD_LIBLCF)
	list(APPEND EASYRPG_PLAYER_LIBRARIES_ALL liblcf)
endif()

# Output path
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Entry point
set(EXE_FILES "src/main.cpp")

# Add prefix (CMAKE_CURRENT_SOURCE_DIR) to list elements
FUNCTION(PREPEND var prefix)
	SET(listVar "")
	FOREACH(f ${ARGN})
		LIST(APPEND listVar "${prefix}/${f}")
	ENDFOREACH(f)
	SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(PLAYER_SRCS ${CMAKE_CURRENT_SOURCE_DIR} ${PLAYER_SRCS})
PREPEND(EXE_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${EXE_FILES})

# Static library
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	list(APPEND PLAYER_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/resources/player.rc")
endif()
add_library(${PROJECT_NAME}_Static STATIC ${PLAYER_SRCS})

# Executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${EXE_FILES})
foreach(i ${PROJECT_NAME} ${PROJECT_NAME})
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
		COMMAND ${A2X_EXECUTABLE} -a player_version="${PACKAGE_VERSION}" -f manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME}.adoc
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

# Doxygen
# Handled as extra target "doc"
find_package(Doxygen)
if(DOXYGEN_FOUND)
	# fake autotools variables
	set(DX_DOCDIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
	set(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
	configure_file(resources/Doxyfile.in resources/Doxyfile @ONLY)
	unset(DX_DOCDIR)
	unset(srcdir)

	add_custom_target(player_doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/resources/Doxyfile
		DEPENDS ${PLAYER_SRCS} ${EXE_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	add_custom_target(doc)
	add_dependencies(doc player_doc)
	if(PLAYER_BUILD_LIBLCF)
		add_dependencies(doc liblcf_doc)
	endif()
endif()

# Unit tests
option(PLAYER_ENABLE_TESTS "Execute unit tests after compilation finishes" ON)

if(PLAYER_ENABLE_TESTS)
	enable_testing()

	set(TEST_GAME_REPOSITORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/TestGame")
	if(NOT EXISTS ${TEST_GAME_REPOSITORY_PATH})
		execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1"
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
if(PLAYER_BUILD_LIBLCF)
	message(STATUS "liblcf: Built from ${PLAYER_BUILD_LIBLCF_GIT}")
	message(STATUS "")
endif()
message(STATUS "Audio backend: ${PLAYER_AUDIO_BACKEND}")
if(${PLAYER_AUDIO_BACKEND} MATCHES "^SDL2.*$")
	message(STATUS "")

	if(SndFile_FOUND)
		message(STATUS "WAV playback:  libsndfile")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "WAV playback:  SDL2_mixer")
	endif()

	if(WildMidi_FOUND)
		if(${PLAYER_ENABLE_FMMIDI} MATCHES "(Fallback|ON)")
			message(STATUS "MIDI playback: WildMidi. Fallback to built-in (FmMidi)")
		else()
			message(STATUS "MIDI playback: WildMidi")
		endif()
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		message(STATUS "MIDI playback: built-in (FmMidi)")
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "Fallback")
		message(STATUS "MIDI playback: SDL2_mixer. Fallback to built-in (FmMidi)")
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		if(SDL2_mixer_FOUND)
			message(STATUS "MIDI playback: SDL2_mixer.")
		else()
			message(STATUS "MIDI playback: None")
		endif()
	endif()

	if(mpg123_FOUND)
		message(STATUS "MP3 playback:  mpg123")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "MP3 playback:  SDL2_mixer. Some MP3 files will crash or play noise.")
	else()
		message(STATUS "MP3 playback:  None")
	endif()

	if(OggVorbis_FOUND)
		message(STATUS "Ogg Vorbis playback: libvorbis")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "Ogg Vorbis playback: SDL2_mixer")
	else()
		message(STATUS "Ogg Vorbis playback: None")
	endif()

	if(XMP_FOUND)
		message(STATUS "MOD playback:  libxmp")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "MOD playback:  SDL2_mixer")
	else()
		message(STATUS "MOD playback:  None")
	endif()

	if(Opus_FOUND)
		message(STATUS "Opus playback: opusfile")
	else()
		message(STATUS "Opus playback: None")
	endif()

	if(speexdsp_FOUND)
		message(STATUS "Resampler:     speexdsp")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "Resampler:     SDL2")
	else()
		message(STATUS "Resampler:     None")
	endif()

	if(SDL_MIXER_USED)
		message(STATUS "")
		message(STATUS "WARNING:")
		message(STATUS "Using SDL2_mixer directly for playback or resampling is not recommended.")
		message(STATUS "Some RPG Maker features (e.g. pitch changes) will not work.")
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

message(STATUS "")
