cmake_minimum_required(VERSION 3.7)

project(EasyRPG_Player VERSION 0.6.1 LANGUAGES CXX)

# C++14 is required
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source Files
add_library(${PROJECT_NAME}
	src/3ds_ui.cpp
	src/3ds_ui.h
	src/async_handler.cpp
	src/async_handler.h
	src/async_op.h
	src/audio_3ds.cpp
	src/audio_3ds.h
	src/audio_al.cpp
	src/audio_al.h
	src/audio.cpp
	src/audio_decoder.cpp
	src/audio_decoder.h
	src/audio_generic.cpp
	src/audio_generic.h
	src/audio.h
	src/audio_psp2.cpp
	src/audio_psp2.h
	src/audio_resampler.cpp
	src/audio_resampler.h
	src/audio_sdl.cpp
	src/audio_sdl.h
	src/audio_sdl_mixer.cpp
	src/audio_sdl_mixer.h
	src/audio_secache.cpp
	src/audio_secache.h
	src/audio_switch.cpp
	src/audio_switch.h
	src/background.cpp
	src/background.h
	src/baseui.cpp
	src/baseui.h
	src/battle_animation.cpp
	src/battle_animation.h
	src/bitmap.cpp
	src/bitmapfont.h
	src/bitmapfont_glyph.h
	src/bitmapfont_rmg2000.h
	src/bitmapfont_ttyp0.h
	src/bitmapfont_wqy.h
	src/bitmap.h
	src/bitmap_hslrgb.h
	src/cache.cpp
	src/cache.h
	src/color.h
	src/decoder_libsndfile.cpp
	src/decoder_libsndfile.h
	src/decoder_mpg123.cpp
	src/decoder_mpg123.h
	src/decoder_oggvorbis.cpp
	src/decoder_oggvorbis.h
	src/decoder_opus.cpp
	src/decoder_opus.h
	src/decoder_wav.cpp
	src/decoder_wav.h
	src/decoder_wildmidi.cpp
	src/decoder_wildmidi.h
	src/decoder_xmp.cpp
	src/decoder_xmp.h
	src/default_graphics.h
	src/dirent_win.h
	src/docmain.h
	src/drawable.cpp
	src/drawable.h
	src/exe_reader.cpp
	src/exe_reader.h
	src/exfont.h
	src/filefinder.cpp
	src/filefinder.h
	src/font.cpp
	src/font.h
	src/fps_overlay.cpp
	src/fps_overlay.h
	src/frame.cpp
	src/frame.h
	src/game_actor.cpp
	src/game_actor.h
	src/game_actors.cpp
	src/game_actors.h
	src/game_battlealgorithm.cpp
	src/game_battlealgorithm.h
	src/game_battle.cpp
	src/game_battle.h
	src/game_battler.cpp
	src/game_battler.h
	src/game_character.cpp
	src/game_character.h
	src/game_commonevent.cpp
	src/game_commonevent.h
	src/game_enemy.cpp
	src/game_enemy.h
	src/game_enemyparty.cpp
	src/game_enemyparty.h
	src/game_event.cpp
	src/game_event.h
	src/game_interpreter_battle.cpp
	src/game_interpreter_battle.h
	src/game_interpreter.cpp
	src/game_interpreter.h
	src/game_interpreter_map.cpp
	src/game_interpreter_map.h
	src/game_map.cpp
	src/game_map.h
	src/game_message.cpp
	src/game_message.h
	src/game_party_base.cpp
	src/game_party_base.h
	src/game_party.cpp
	src/game_party.h
	src/game_picture.cpp
	src/game_picture.h
	src/game_player.cpp
	src/game_player.h
	src/game_screen.cpp
	src/game_screen.h
	src/game_switches.cpp
	src/game_switches.h
	src/game_system.cpp
	src/game_system.h
	src/game_targets.cpp
	src/game_targets.h
	src/game_temp.cpp
	src/game_temp.h
	src/game_variables.cpp
	src/game_variables.h
	src/game_vehicle.cpp
	src/game_vehicle.h
	src/graphics.cpp
	src/graphics.h
	src/hslrgb.cpp
	src/hslrgb.h
	src/icon.h
	src/image_bmp.cpp
	src/image_bmp.h
	src/image_png.cpp
	src/image_png.h
	src/image_xyz.cpp
	src/image_xyz.h
	src/input_buttons_desktop.cpp
	src/input_buttons_gekko.cpp
	src/input_buttons.h
	src/input_buttons_opendingux.cpp
	src/input_buttons_psp.cpp
	src/input.cpp
	src/input.h
	src/input_source.cpp
	src/input_source.h
	src/keys.h
	src/logo.h
	src/main_data.cpp
	src/main_data.h
	src/map_data.h
	src/memory_management.h
	src/message_overlay.cpp
	src/message_overlay.h
	src/meta.cpp
	src/meta.h
	src/options.h
	src/output.cpp
	src/output.h
	src/picojson.h
	src/pixel_format.h
	src/plane.cpp
	src/plane.h
	src/player.cpp
	src/player.h
	src/psp2_ui.cpp
	src/psp2_ui.h
	src/rect.cpp
	src/rect.h
	src/registry.cpp
	src/registry.h
	src/registry_wine.cpp
	src/rtp.cpp
	src/rtp.h
	src/rtp_table.cpp
	src/scene_actortarget.cpp
	src/scene_actortarget.h
	src/scene_battle.cpp
	src/scene_battle.h
	src/scene_battle_rpg2k3.cpp
	src/scene_battle_rpg2k3.h
	src/scene_battle_rpg2k.cpp
	src/scene_battle_rpg2k.h
	src/scene.cpp
	src/scene_debug.cpp
	src/scene_debug.h
	src/scene_end.cpp
	src/scene_end.h
	src/scene_equip.cpp
	src/scene_equip.h
	src/scene_file.cpp
	src/scene_file.h
	src/scene_gamebrowser.cpp
	src/scene_gamebrowser.h
	src/scene_gameover.cpp
	src/scene_gameover.h
	src/scene.h
	src/scene_import.cpp
	src/scene_import.h
	src/scene_item.cpp
	src/scene_item.h
	src/scene_load.cpp
	src/scene_load.h
	src/scene_logo.cpp
	src/scene_logo.h
	src/scene_map.cpp
	src/scene_map.h
	src/scene_menu.cpp
	src/scene_menu.h
	src/scene_name.cpp
	src/scene_name.h
	src/scene_order.cpp
	src/scene_order.h
	src/scene_save.cpp
	src/scene_save.h
	src/scene_shop.cpp
	src/scene_shop.h
	src/scene_skill.cpp
	src/scene_skill.h
	src/scene_status.cpp
	src/scene_status.h
	src/scene_teleport.cpp
	src/scene_teleport.h
	src/scene_title.cpp
	src/scene_title.h
	src/screen.cpp
	src/screen.h
	src/shinonome_gothic.h
	src/shinonome_mincho.h
	src/sprite_airshipshadow.cpp
	src/sprite_airshipshadow.h
	src/sprite_battler.cpp
	src/sprite_battler.h
	src/sprite_character.cpp
	src/sprite_character.h
	src/sprite.cpp
	src/sprite.h
	src/spriteset_battle.cpp
	src/spriteset_battle.h
	src/spriteset_map.cpp
	src/spriteset_map.h
	src/sprite_timer.cpp
	src/sprite_timer.h
	src/state.cpp
	src/state.h
	src/switch_ui.cpp
	src/switch_ui.h
	src/system.h
	src/teleport_target.h
	src/text.cpp
	src/text.h
	src/tilemap.cpp
	src/tilemap.h
	src/tilemap_layer.cpp
	src/tilemap_layer.h
	src/tone.h
	src/transform.h
	src/transition.cpp
	src/transition.h
	src/util_macro.h
	src/utils.cpp
	src/utils.h
	src/util_win.cpp
	src/util_win.h
	src/version.h
	src/weather.cpp
	src/weather.h
	src/window_about.cpp
	src/window_about.h
	src/window_actorinfo.cpp
	src/window_actorinfo.h
	src/window_actorsp.cpp
	src/window_actorsp.h
	src/window_actorstatus.cpp
	src/window_actorstatus.h
	src/window_actortarget.cpp
	src/window_actortarget.h
	src/window_base.cpp
	src/window_base.h
	src/window_battlecommand.cpp
	src/window_battlecommand.h
	src/window_battlemessage.cpp
	src/window_battlemessage.h
	src/window_battleoption.cpp
	src/window_battleoption.h
	src/window_battlestatus.cpp
	src/window_battlestatus.h
	src/window_command.cpp
	src/window_command.h
	src/window.cpp
	src/window_equip.cpp
	src/window_equip.h
	src/window_equipitem.cpp
	src/window_equipitem.h
	src/window_equipstatus.cpp
	src/window_equipstatus.h
	src/window_face.cpp
	src/window_face.h
	src/window_gamelist.cpp
	src/window_gamelist.h
	src/window_gold.cpp
	src/window_gold.h
	src/window.h
	src/window_help.cpp
	src/window_help.h
	src/window_import_progress.cpp
	src/window_import_progress.h
	src/window_item.cpp
	src/window_item.h
	src/window_keyboard.cpp
	src/window_keyboard.h
	src/window_menustatus.cpp
	src/window_menustatus.h
	src/window_message.cpp
	src/window_message.h
	src/window_name.cpp
	src/window_name.h
	src/window_numberinput.cpp
	src/window_numberinput.h
	src/window_paramstatus.cpp
	src/window_paramstatus.h
	src/window_savefile.cpp
	src/window_savefile.h
	src/window_selectable.cpp
	src/window_selectable.h
	src/window_shopbuy.cpp
	src/window_shopbuy.h
	src/window_shop.cpp
	src/window_shop.h
	src/window_shopnumber.cpp
	src/window_shopnumber.h
	src/window_shopparty.cpp
	src/window_shopparty.h
	src/window_shopsell.cpp
	src/window_shopsell.h
	src/window_shopstatus.cpp
	src/window_shopstatus.h
	src/window_skill.cpp
	src/window_skill.h
	src/window_skillstatus.cpp
	src/window_skillstatus.h
	src/window_targetstatus.cpp
	src/window_targetstatus.h
	src/window_teleport.cpp
	src/window_teleport.h
	src/window_varlist.cpp
	src/window_varlist.h
)

include(CMakeDependentOption)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Extra CMake Module files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")

include(ConfigureWindows)
include(PlayerFindPackage)

# Platform setup
set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "Platform to compile for. Options: SDL2 libretro")
set_property(CACHE PLAYER_TARGET_PLATFORM PROPERTY STRINGS SDL2 libretro)
set(PLAYER_BUILD_EXECUTABLE ON)

if(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2")
	target_sources(${PROJECT_NAME} PRIVATE
		src/sdl_ui.cpp
		src/sdl_ui.h
		src/sdl2_ui.cpp
		src/sdl2_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=2)

	find_package(SDL2 REQUIRED)
	target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "libretro")
	target_sources(${PROJECT_NAME} PRIVATE
		src/audio_libretro.cpp
		src/audio_libretro.h
		src/input_buttons_libretro.cpp
		src/libretro_ui.cpp
		src/libretro_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_LIBRETRO=1)
	set(PLAYER_BUILD_EXECUTABLE OFF)
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "easyrpg_libretro")

	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/builds/libretro)
	target_link_libraries(${PROJECT_NAME} retro_common)
else()
	message(FATAL_ERROR "Invalid target platform")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${PROJECT_NAME} PUBLIC _DEBUG=1)
endif()

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
	if(NOT EXISTS ${LIBLCF_PATH})
		find_package(Git REQUIRED)
		execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1"
			"${PLAYER_BUILD_LIBLCF_GIT}"
			"${LIBLCF_PATH}")
	endif()
	SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build liblcf shared library")
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules")

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

# Always enable Wine registry support on non-Windows
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_WINE=1)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_HARFBUZZ "Enable HarfBuzz text shaping (Requires FreeType)" ON "PLAYER_WITH_FREETYPE" OFF)

player_find_package(NAME Freetype
	CONDITION PLAYER_WITH_FREETYPE
	DEFINITION HAVE_FREETYPE
	FOUND Freetype
	TARGET Freetype::Freetype)

if(Freetype_FOUND)
	player_find_package(NAME Harfbuzz
		CONDITION PLAYER_WITH_HARFBUZZ
		DEFINITION HAVE_HARFBUZZ
		TARGET Harfbuzz::Harfbuzz)
endif()

# Sound system to use
if(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2")
	set(PLAYER_AUDIO_BACKEND "Auto" CACHE STRING "Audio system to use. Options: Auto SDL2_mixer SDL2 OpenAL OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS Auto SDL2 SDL2_mixer OpenAL OFF)

	# Configure Audio backends
	if(${PLAYER_AUDIO_BACKEND} STREQUAL "Auto")
		set(PLAYER_AUDIO_BACKEND_IS_AUTO ON)
	endif()
	CMAKE_DEPENDENT_OPTION(PLAYER_WITH_SDL2_MIXER "Use SDL2 mixer when available" ON "PLAYER_AUDIO_BACKEND_IS_AUTO" OFF)

	if(PLAYER_AUDIO_BACKEND_IS_AUTO)
		player_find_package(NAME SDL2_mixer
			CONDITION PLAYER_WITH_SDL2_MIXER
			TARGET SDL2::MIXER)
		if(SDL2_MIXER_FOUND)
			set(PLAYER_AUDIO_BACKEND "SDL2_mixer")
		else()
			set(PLAYER_AUDIO_BACKEND "SDL2")
		endif()
	endif()
else()
	set(PLAYER_AUDIO_BACKEND "Default" CACHE STRING "Audio system to use. Options: Default OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS Default OFF)

	if(${PLAYER_AUDIO_BACKEND} STREQUAL "Default")
		# Assuming that all platforms not targeting SDL have only one audio backend
		set(PLAYER_AUDIO_BACKEND ${PLAYER_TARGET_PLATFORM})
	endif()
endif()

# Configure Audio backends
if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL2|libretro)$")
	set(PLAYER_HAS_AUDIO ON)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	# Provide fmmidi options
	option(PLAYER_ENABLE_FMMIDI "Enable internal MIDI sequencer. Will be used when external MIDI library fails." ON)
	if(PLAYER_ENABLE_FMMIDI)
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=1)
	endif()
elseif(${PLAYER_AUDIO_BACKEND} STREQUAL "SDL2_mixer")
	set(PLAYER_HAS_AUDIO ON)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	# SDL2_mixer Audio
	player_find_package(NAME SDL2_mixer
		DEFINITION HAVE_SDL_MIXER
		TARGET SDL2::MIXER
		REQUIRED)

	# Provide fmmidi options
	set(PLAYER_ENABLE_FMMIDI "Fallback" CACHE STRING "Enable internal MIDI sequencer. Fallback (default) mode will use it when the external MIDI library fails. Options: Fallback ON OFF")
	set_property(CACHE PLAYER_ENABLE_FMMIDI PROPERTY STRINGS Fallback ON OFF)
	if(${PLAYER_ENABLE_FMMIDI} STREQUAL "Fallback")
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=2)
	elseif(${PLAYER_ENABLE_FMMIDI} STREQUAL "ON")
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=1)
	elseif(${PLAYER_ENABLE_FMMIDI} STREQUAL "OFF")
		# noop
	else()
		message(FATAL_ERROR "Bad FmMidi setting ${PLAYER_ENABLE_FMMIDI}")
	endif()
elseif(${PLAYER_AUDIO_BACKEND} STREQUAL "OpenAL")
	# OpenAL Audio
	set(PLAYER_HAS_AUDIO ON)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	find_package(OpenAL REQUIRED)
	find_package(LibSndFile REQUIRED)
	find_package(FluidSynth REQUIRED)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENAL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY} LibSndFile::LibSndFile fluidsynth::fluidsynth)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_OPENAL=1)
elseif(NOT PLAYER_AUDIO_BACKEND)
	set(PLAYER_HAS_AUDIO OFF)
else()
	message(FATAL_ERROR "Invalid Audio Backend ${PLAYER_AUDIO_BACKEND}")
endif()

CMAKE_DEPENDENT_OPTION(PLAYER_WITH_MPG123 "Play MP3 audio with libmpg123" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_LIBSNDFILE "Play WAV audio with libsndfile" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_OGGVORBIS "Play Ogg Vorbis audio with libvorbis" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_OPUS "Play Opus audio with opusfile" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_WILDMIDI "Play MIDI audio with wildmidi" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_XMP "Play MOD audio with libxmp" ON "PLAYER_HAS_AUDIO" OFF)

if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL.*|libretro)$")
	set(PLAYER_AUDIO_RESAMPLER "Auto" CACHE STRING "Audio resampler to use. Options: Auto speexdsp samplerate OFF")
	set_property(CACHE PLAYER_AUDIO_RESAMPLER PROPERTY STRINGS Auto speexdsp samplerate OFF)

	if(${PLAYER_AUDIO_RESAMPLER} STREQUAL "Auto")
		set(PLAYER_AUDIO_RESAMPLER_IS_AUTO ON)
	endif()

	CMAKE_DEPENDENT_OPTION(PLAYER_WITH_SPEEXDSP "Use speexdsp as resampler when available." ON "PLAYER_HAS_AUDIO;PLAYER_AUDIO_RESAMPLER_IS_AUTO" OFF)
	CMAKE_DEPENDENT_OPTION(PLAYER_WITH_SAMPLERATE "Use libsamplerate as resampler when available." ON "PLAYER_HAS_AUDIO;PLAYER_AUDIO_RESAMPLER_IS_AUTO" OFF)

	if(PLAYER_AUDIO_RESAMPLER_IS_AUTO)
		player_find_package(NAME speexdsp
			CONDITION PLAYER_WITH_SPEEXDSP
			DEFINITION HAVE_LIBSPEEXDSP
			TARGET speexdsp::speexdsp)
		if(NOT SPEEXDSP_FOUND)
			player_find_package(
				NAME Samplerate
				CONDITION PLAYER_WITH_SAMPLERATE
				DEFINITION HAVE_LIBSAMPLERATE
				TARGET Samplerate::Samplerate)
		endif()
	elseif(${PLAYER_AUDIO_RESAMPLER} STREQUAL "speexdsp")
		player_find_package(NAME speexdsp
			DEFINITION HAVE_LIBSPEEXDSP
			TARGET speexdsp::speexdsp
			REQUIRED)
	elseif(${PLAYER_AUDIO_RESAMPLER} STREQUAL "samplerate")
		player_find_package(NAME Samplerate
			DEFINITION HAVE_LIBSAMPLERATE
			TARGET Samplerate::Samplerate
			REQUIRED)
	elseif(NOT PLAYER_AUDIO_RESAMPLER)
		# no-op
	else()
		message(FATAL_ERROR "Invalid Audio Resampler ${PLAYER_AUDIO_RESAMPLER}")
	endif()

	# mpg123
	player_find_package(NAME mpg123
		CONDITION PLAYER_WITH_MPG123
		DEFINITION HAVE_MPG123
		TARGET mpg123::mpg123)

	# libsndfile
	player_find_package(NAME LibSndFile
		CONDITION PLAYER_WITH_LIBSNDFILE
		DEFINITION HAVE_LIBSNDFILE
		TARGET LibSndFile::LibSndFile)

	# libvorbis
	player_find_package(NAME Vorbisfile
		CONDITION PLAYER_WITH_OGGVORBIS
		DEFINITION HAVE_OGGVORBIS
		TARGET Vorbisfile::Vorbisfile)

	# opusfile
	player_find_package(NAME Opusfile
		CONDITION PLAYER_WITH_OPUS
		DEFINITION HAVE_OPUS
		TARGET Opusfile::Opusfile)

	# wildmidi
	player_find_package(NAME WildMidi
		CONDITION PLAYER_WITH_WILDMIDI
		DEFINITION HAVE_WILDMIDI
		TARGET WildMidi::WildMidi)

	# xmp (lite)
	player_find_package(NAME XMP
		CONDITION PLAYER_WITH_XMP
		DEFINITION HAVE_XMP
		TARGET XMP::XMP)

	if(PLAYER_ENABLE_FMMIDI)
		target_sources(${PROJECT_NAME} PRIVATE
			src/decoder_fmmidi.cpp
			src/decoder_fmmidi.h
			src/midiprogram.h
			src/midisequencer.cpp
			src/midisequencer.h
			src/midisynth.cpp
			src/midisynth.h)
	endif()
endif()

# MSVC: Set UTF-8 encoding for all files (important for rtp table)
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/utf-8")
endif()

# Executable
if (${PLAYER_BUILD_EXECUTABLE})
	add_executable(${PROJECT_NAME}_exe WIN32 "src/main.cpp")
	set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "easyrpg-player")

	if(WIN32)
		# Open console for Debug builds
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			set_target_properties(${PROJECT_NAME}_exe PROPERTIES WIN32_EXECUTABLE FALSE)
		endif()

		# Add resources
		target_sources(${PROJECT_NAME}_exe PRIVATE "resources/player.rc")

		# Change executable name
		set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "Player")
	endif()

	target_link_libraries(${PROJECT_NAME}_exe ${PROJECT_NAME})

	# installation
	include(GNUInstallDirs)
	install(TARGETS ${PROJECT_NAME}_exe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}_exe> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
	endif()
endif()

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
	if(UNIX)
		add_custom_target(man ALL DEPENDS resources/${MAN_NAME})
		install(FILES resources/${MAN_NAME} DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
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

# Benchmarks
option(PLAYER_ENABLE_BENCHMARKS "Build benchmarks" OFF)

if(PLAYER_ENABLE_BENCHMARKS)
	find_package(benchmark REQUIRED)
	file(GLOB BENCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bench/*.cpp)
	foreach(i ${BENCH_FILES})
		get_filename_component(name "${i}" NAME_WE)
		add_executable(bench_${name} ${i})
		target_link_libraries(bench_${name} ${PROJECT_NAME})
		target_link_libraries(bench_${name} benchmark)
	endforeach()
endif()



# Print summary
message(STATUS "")
message(STATUS "Target system: ${PLAYER_TARGET_PLATFORM}")
message(STATUS "")

if(PLAYER_BUILD_LIBLCF)
	message(STATUS "liblcf: Built from ${PLAYER_BUILD_LIBLCF_GIT}")
	message(STATUS "")
endif()

message(STATUS "Audio backend: ${PLAYER_AUDIO_BACKEND}")
if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL.*|libretro)$")
	message(STATUS "")

	if(LIBSNDFILE_FOUND)
		message(STATUS "WAV playback: libsndfile")
	elseif(SDL2_MIXER_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "WAV playback: SDL2_mixer")
	else()
		message(STATUS "WAV playback: None")
	endif()

	if(WILDMIDI_FOUND)
		if(${PLAYER_ENABLE_FMMIDI} MATCHES "(Fallback|ON)")
			message(STATUS "MIDI playback: WildMidi. Fallback to built-in (FmMidi)")
		else()
			message(STATUS "MIDI playback: WildMidi")
		endif()
	elseif(${PLAYER_ENABLE_FMMIDI} STREQUAL "Fallback")
		message(STATUS "MIDI playback: SDL2_mixer. Fallback to built-in (FmMidi)")
	elseif(PLAYER_ENABLE_FMMIDI)
		message(STATUS "MIDI playback: built-in (FmMidi)")
	else()
		if(SDL2_MIXER_FOUND)
			message(STATUS "MIDI playback: SDL2_mixer.")
		else()
			message(STATUS "MIDI playback: None")
		endif()
	endif()

	if(MPG123_FOUND)
		message(STATUS "MP3 playback: mpg123")
	elseif(SDL2_MIXER_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "MP3 playback: SDL2_mixer")
	else()
		message(STATUS "MP3 playback: None")
	endif()

	if(VORBISFILE_FOUND)
		message(STATUS "Ogg Vorbis playback: libvorbis")
	elseif(SDL2_MIXER_FOUND)
		set(SDL_MIXER_USED ON)
		message(STATUS "Ogg Vorbis playback: SDL2_mixer")
	else()
		message(STATUS "Ogg Vorbis playback: None")
	endif()

	if(XMP_FOUND)
		message(STATUS "MOD playback: libxmp")
	elseif(SDL2_MIXER_FOUND)
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
	elseif(SAMPLERATE_FOUND)
		message(STATUS "Resampler: libsamplerate")
	elseif(SDL2_MIXER_FOUND)
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
