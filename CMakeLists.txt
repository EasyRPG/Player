cmake_minimum_required(VERSION 3.7)

project(EasyRPG_Player VERSION 0.5.4 LANGUAGES CXX)

# Source Files
add_library(${PROJECT_NAME}
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
	src/drawable.cpp
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
	src/sdl2_ui.cpp
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
	src/transition.cpp
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

include(CMakeDependentOption)

# Include directories
target_include_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src)

# Extra CMake Module files
list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")

include(ConfigureWindows)
include(PlayerFindPackage)

# C++11 is required
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	target_compile_definitions(${PROJECT_NAME} PUBLIC _DEBUG=1)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=2)

# Endianess check
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	include(TestBigEndian)
	test_big_endian(WORDS_BIGENDIAN)
	if(WORDS_BIGENDIAN)
		target_compile_definitions(${PROJECT_NAME} PUBLIC WORDS_BIGENDIAN=1)
	endif()
endif()

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
	SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build liblcf shared library")
	list(APPEND CMAKE_MODULE_PATH
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules")

	add_subdirectory(${LIBLCF_PATH})
	target_link_libraries(${PROJECT_NAME} lcf)
else()
	# Use system package
	find_package(liblcf REQUIRED)
	target_link_libraries(${PROJECT_NAME} liblcf::liblcf)
endif()

# Detect all required libraries
find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PNG::PNG)

find_package(Pixman REQUIRED)
target_link_libraries(${PROJECT_NAME} PIXMAN::PIXMAN)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)

# Always enable Wine registry support on non-Windows
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_WINE=1)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_HARFBUZZ "Enable HarfBuzz text shaping (Requires FreeType)" ON "PLAYER_WITH_FREETYPE" OFF)

player_find_package(
	NAME Freetype
	CONDITION PLAYER_WITH_FREETYPE
	DEFINITION HAVE_FREETYPE
	FOUND Freetype
	TARGET Freetype::Freetype)

if(Freetype_FOUND)
	player_find_package(
		NAME Harfbuzz
		CONDITION PLAYER_WITH_HARFBUZZ
		DEFINITION HAVE_HARFBUZZ
		TARGET Harfbuzz::Harfbuzz)
endif()

# Sound system to use
set(PLAYER_AUDIO_BACKEND "Auto" CACHE STRING "Audio system to use. Options: Auto SDL2_mixer SDL2 OpenAL OFF")
set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS Auto SDL2 SDL2_mixer OpenAL OFF)

# Configure Audio backends
if(${PLAYER_AUDIO_BACKEND} MATCHES "Auto")
	option(PLAYER_WITH_SDL2_MIXER "Use SDL2 mixer when available." ON)
	player_find_package(
		NAME SDL2_mixer
		CONDITION PLAYER_WITH_SDL2_MIXER
		TARGET SDL2::MIXER)
	if(SDL2_MIXER_FOUND)
		set(PLAYER_AUDIO_BACKEND "SDL2_mixer")
	else()
		set(PLAYER_AUDIO_BACKEND "SDL2")
	endif()
endif()

if(${PLAYER_AUDIO_BACKEND} MATCHES "^SDL2$")
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	# Provide fmmidi options
	option(PLAYER_ENABLE_FMMIDI "Enable internal MIDI sequencer. Will be used when external MIDI library fails." ON)
	if(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=1)
	endif()
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "SDL2_mixer")
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	# SDL2_mixer Audio
	player_find_package(
		NAME SDL2_mixer
		DEFINITION HAVE_SDL_MIXER
		TARGET SDL2::MIXER
		REQUIRED)

	# Provide fmmidi options
	set(PLAYER_ENABLE_FMMIDI "Fallback" CACHE STRING "Enable internal MIDI sequencer. Fallback (default) mode will use it when the external MIDI library fails. Options: Fallback ON OFF")
	set_property(CACHE PLAYER_ENABLE_FMMIDI PROPERTY STRINGS Fallback ON OFF)
	if(${PLAYER_ENABLE_FMMIDI} MATCHES "Fallback")
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=2)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "ON")
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=1)
	elseif(${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		# noop
	else()
		message(FATAL_ERROR "Bad FmMidi setting")
	endif()
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OpenAL")
	# OpenAL Audio
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	find_package(OpenAL REQUIRED)
	find_package(LibSndFile REQUIRED)
	find_package(FluidSynth REQUIRED)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENAL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY} LibSndFile::LibSndFile Fluidsynth::Fluidsynth)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_OPENAL=1)
elseif(${PLAYER_AUDIO_BACKEND} MATCHES "OFF")

else()
	message(FATAL_ERROR "Invalid Audio Backend")
endif()

if(${PLAYER_AUDIO_BACKEND} MATCHES "^SDL.*$")
	# speexdsp
	option(PLAYER_WITH_SPEEXDSP "Resample audio with speexdsp." ON)
	player_find_package(
		NAME speexdsp
		CONDITION PLAYER_WITH_SPEEXDSP
		DEFINITION HAVE_LIBSPEEXDSP
		TARGET speexdsp::speexdsp)

	# mpg123
	option(PLAYER_WITH_MPG123 "Play MP3 audio with libmpg123." ON)
	player_find_package(
		NAME mpg123
		CONDITION PLAYER_WITH_MPG123
		DEFINITION HAVE_MPG123
		TARGET mpg123::mpg123)

	# libsndfile
	option(PLAYER_WITH_LIBSNDFILE "Play WAV audio with libsndfile." ON)
	player_find_package(
		NAME LibSndFile
		CONDITION PLAYER_WITH_LIBSNDFILE
		DEFINITION HAVE_LIBSNDFILE
		TARGET LibSndFile::LibSndFile)

	# libvorbis
	option(PLAYER_WITH_OGGVORBIS "Play Ogg Vorbis audio with libvorbis." ON)
	player_find_package(
		NAME Vorbisfile
		CONDITION PLAYER_WITH_OGGVORBIS
		DEFINITION HAVE_OGGVORBIS
		TARGET Vorbisfile::Vorbisfile)

	# opusfile
	option(PLAYER_WITH_OPUS "Play Opus audio with opusfile." ON)
	player_find_package(
		NAME Opusfile
		CONDITION PLAYER_WITH_OPUS
		DEFINITION HAVE_OPUS
		TARGET Opusfile::Opusfile)

	# wildmidi
	option(PLAYER_WITH_WILDMIDI "Play MIDI audio with wildmidi." ON)
	player_find_package(
		NAME WildMidi
		CONDITION PLAYER_WITH_WILDMIDI
		DEFINITION HAVE_WILDMIDI
		TARGET WildMidi::WildMidi)

	# xmp (lite)
	option(PLAYER_WITH_XMP "Play MOD audio with libxmp." ON)
	player_find_package(
		NAME XMP
		CONDITION PLAYER_WITH_XMP
		DEFINITION HAVE_XMP
		TARGET XMP::XMP)

	if(NOT ${PLAYER_ENABLE_FMMIDI} MATCHES "OFF")
		target_sources(${PROJECT_NAME} PRIVATE
			src/midisequencer.cpp
			src/midisynth.cpp)
	endif()
endif()

# Executable
# Windows: Only open console for Debug builds
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_executable(${PROJECT_NAME}_exe "src/main.cpp")
else()
	add_executable(${PROJECT_NAME}_exe WIN32 "src/main.cpp")
endif()

if(WIN32)
	set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "Player")
else()
	set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "easyrpg-player")
endif()

# Add Windows resources
if(WIN32)
	target_sources(${PROJECT_NAME}_exe PRIVATE "resources/player.rc")
endif()

target_link_libraries(${PROJECT_NAME}_exe ${PROJECT_NAME})

# installation
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}_exe
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# manpage
# Handled as extra target "man"
set(MAN_NAME easyrpg-player.6)
find_program(A2X_EXECUTABLE NAMES a2x a2x.py)
if(NOT A2X_EXECUTABLE STREQUAL "A2X_EXECUTABLE-NOTFOUND")
	add_custom_command(OUTPUT resources/${MAN_NAME}
		COMMAND ${A2X_EXECUTABLE} -a player_version="${PROJECT_VERSION}" -f manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/${MAN_NAME}.adoc
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
		DEPENDS ${PROJECT_NAME}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	if(NOT TARGET doc)
		# Handle liblcf doc target
		add_custom_target(doc)
	endif()
	add_dependencies(doc player_doc)
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
		target_link_libraries(test_${name} ${PROJECT_NAME})

		add_test(
			NAME test_${name} WORKING_DIRECTORY ${TEST_GAME_PATH}
			COMMAND ${PROJECT_BINARY_DIR}/test_${name})
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

	if(LIBSNDFILE_FOUND)
		message(STATUS "WAV playback: libsndfile")
	else()
		set(SDL_MIXER_USED ON)
		message(STATUS "WAV playback: SDL2_mixer")
	endif()

	if(WILDMIDI_FOUND)
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

	if(MPG123_FOUND)
		message(STATUS "MP3 playback: mpg123")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "MP3 playback: SDL2_mixer")
	else()
		message(STATUS "MP3 playback: None")
	endif()

	if(VORBISFILE_FOUND)
		message(STATUS "Ogg Vorbis playback: libvorbis")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "Ogg Vorbis playback: SDL2_mixer")
	else()
		message(STATUS "Ogg Vorbis playback: None")
	endif()

	if(XMP_FOUND)
		message(STATUS "MOD playback: libxmp")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "MOD playback: SDL2_mixer")
	else()
		message(STATUS "MOD playback: None")
	endif()

	if(OPUSFILE_FOUND)
		message(STATUS "Opus playback: opusfile")
	else()
		message(STATUS "Opus playback: None")
	endif()

	if(SPEEXDSP_FOUND)
		message(STATUS "Resampler: speexdsp")
	elseif(SDL2_mixer_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "Resampler: SDL2")
	else()
		message(STATUS "Resampler: None")
	endif()

	if(SDL_MIXER_USED)
		message(STATUS "")
		message(STATUS "WARNING:")
		message(STATUS "Using SDL2_mixer directly for playback or resampling is not recommended.")
		message(STATUS "Some RPG Maker features (e.g. pitch changes) will not work.")
	endif()
endif()

message(STATUS "")

if(HARFBUZZ_FOUND)
	message(STATUS "Font rendering: Freetype with Harfbuzz / built-in")
elseif(Freetype_FOUND)
	message(STATUS "Font rendering: Freetype and built-in")
else()
	message(STATUS "Font rendering: built-in")
endif()

message(STATUS "")
