cmake_minimum_required(VERSION 3.13...3.24 FATAL_ERROR)

project(EasyRPG_Player VERSION 0.8
	DESCRIPTION "Interpreter for RPG Maker 2000/2003 games"
	HOMEPAGE_URL "https://easyrpg.org"
	LANGUAGES CXX)

# Extra CMake Module files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")
include(CMakeDependentOption)
include(PlayerConfigureWindows)
include(PlayerFindPackage)
include(PlayerBuildType)
include(PlayerMisc)
include(GetGitRevisionDescription)

# C++17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Must be at global scope, otherwise breaks -DPLAYER_BUILD_LIBLCF (see CMP0077)
option(BUILD_SHARED_LIBS "Build shared easyrpg_libretro core" ON)

option(BUILD_CLIENT "Build client" ON)
option(BUILD_SERVER "Build server" OFF)

if(NOT BUILD_CLIENT AND NOT BUILD_SERVER)
	set(BUILD_CLIENT on)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.15.0")
	option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Prefer config files over bundled FindXXX files. Set this to OFF when configuration fails and report a bug." ON)
endif()

# Source Files
add_library(${PROJECT_NAME} OBJECT
	src/lcf_data.cpp
	src/lcf/data.h
	src/async_handler.cpp
	src/async_handler.h
	src/async_op.h
	src/algo.h
	src/algo.cpp
	src/attribute.h
	src/attribute.cpp
	src/audio.cpp
	src/audio_decoder.cpp
	src/audio_decoder.h
	src/audio_decoder_base.cpp
	src/audio_decoder_base.h
	src/audio_decoder_midi.cpp
	src/audio_decoder_midi.h
	src/audio_generic.cpp
	src/audio_generic.h
	src/audio_generic_midiout.cpp
	src/audio_generic_midiout.h
	src/audio.h
	src/audio_midi.cpp
	src/audio_midi.h
	src/audio_resampler.cpp
	src/audio_resampler.h
	src/audio_secache.cpp
	src/audio_secache.h
	src/autobattle.cpp
	src/autobattle.h
	src/background.cpp
	src/background.h
	src/baseui.cpp
	src/baseui.h
	src/battle_animation.cpp
	src/battle_animation.h
	src/battle_message.cpp
	src/battle_message.h
	src/bitmap.cpp
	src/bitmapfont.h
	src/bitmapfont_glyph.h
	src/bitmap.h
	src/bitmap_hslrgb.h
	src/cache.cpp
	src/cache.h
	src/cmdline_parser.cpp
	src/cmdline_parser.h
	src/color.h
	src/compiler.h
	src/config_param.h
	src/decoder_fluidsynth.cpp
	src/decoder_fluidsynth.h
	src/decoder_libsndfile.cpp
	src/decoder_libsndfile.h
	src/decoder_mpg123.cpp
	src/decoder_mpg123.h
	src/decoder_oggvorbis.cpp
	src/decoder_oggvorbis.h
	src/decoder_opus.cpp
	src/decoder_opus.h
	src/decoder_wildmidi.cpp
	src/decoder_wildmidi.h
	src/decoder_xmp.cpp
	src/decoder_xmp.h
	src/default_graphics.h
	src/directory_tree.cpp
	src/directory_tree.h
	src/docmain.h
	src/drawable.cpp
	src/drawable.h
	src/drawable_list.cpp
	src/drawable_list.h
	src/drawable_mgr.cpp
	src/drawable_mgr.h
	src/dynrpg.cpp
	src/dynrpg.h
	src/dynrpg_easyrpg.cpp
	src/dynrpg_easyrpg.h
	src/enemyai.cpp
	src/enemyai.h
	src/exe_reader.cpp
	src/exe_reader.h
	src/exfont.h
	src/feature.cpp
	src/feature.h
	src/filefinder.cpp
	src/filefinder.h
	src/filefinder_rtp.cpp
	src/filefinder_rtp.h
	src/fileext_guesser.cpp
	src/fileext_guesser.h
	src/filesystem.cpp
	src/filesystem.h
	src/filesystem_native.cpp
	src/filesystem_native.h
	src/filesystem_root.cpp
	src/filesystem_root.h
	src/filesystem_stream.cpp
	src/filesystem_stream.h
	src/filesystem_zip.cpp
	src/filesystem_zip.h
	src/flash.h
	src/flat_map.h
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
	src/game_clock.cpp
	src/game_clock.h
	src/game_commonevent.cpp
	src/game_commonevent.h
	src/game_config.cpp
	src/game_config.h
	src/game_config_game.cpp
	src/game_config_game.h
	src/game_enemy.cpp
	src/game_enemy.h
	src/game_enemyparty.cpp
	src/game_enemyparty.h
	src/game_event.cpp
	src/game_event.h
	src/game_ineluki.cpp
	src/game_ineluki.h
	src/game_interpreter_battle.cpp
	src/game_interpreter_battle.h
	src/game_interpreter_control_variables.cpp
	src/game_interpreter_control_variables.h
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
	src/game_pictures.cpp
	src/game_pictures.h
	src/game_player.cpp
	src/game_player.h
	src/game_quit.cpp
	src/game_quit.h
	src/game_screen.cpp
	src/game_screen.h
	src/game_switches.cpp
	src/game_switches.h
	src/game_system.cpp
	src/game_system.h
	src/game_targets.cpp
	src/game_targets.h
	src/game_variables.cpp
	src/game_variables.h
	src/game_vehicle.cpp
	src/game_vehicle.h
	src/game_windows.cpp
	src/game_windows.h
	src/generated/bitmapfont_rmg2000.h
	src/generated/bitmapfont_ttyp0.h
	src/generated/bitmapfont_wqy.h
	src/generated/logo.h
	src/generated/logo2.h
	src/generated/shinonome_gothic.h
	src/generated/shinonome_mincho.h
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
	src/input_buttons.h
	src/input.cpp
	src/input.h
	src/input_source.cpp
	src/input_source.h
	src/instrumentation.cpp
	src/instrumentation.h
	src/keys.h
	src/main_data.cpp
	src/main_data.h
	src/maniac_patch.cpp
	src/maniac_patch.h
	src/map_data.h
	src/memory_management.h
	src/message_overlay.cpp
	src/message_overlay.h
	src/meta.cpp
	src/meta.h
	src/midisequencer.cpp
	src/midisequencer.h
	src/opacity.h
	src/options.h
	src/output.cpp
	src/output.h
	src/pending_message.h
	src/pending_message.cpp
	src/pixel_format.h
	src/pixman_image_ptr.h
	src/plane.cpp
	src/plane.h
	src/platform.cpp
	src/platform.h
	src/platform/clock.h
	src/player.cpp
	src/player.h
	src/point.h
	src/rand.cpp
	src/rand.h
	src/rect.cpp
	src/rect.h
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
	src/scene_settings.h
	src/scene_settings.cpp
	src/scene_teleport.cpp
	src/scene_teleport.h
	src/scene_title.cpp
	src/scene_title.h
	src/screen.cpp
	src/screen.h
	src/shake.h
	src/span.h
	src/sprite_airshipshadow.cpp
	src/sprite_airshipshadow.h
	src/sprite_actor.cpp
	src/sprite_actor.h
	src/sprite_battler.cpp
	src/sprite_battler.h
	src/sprite_enemy.cpp
	src/sprite_enemy.h
	src/sprite_character.cpp
	src/sprite_character.h
	src/sprite_picture.h
	src/sprite_picture.cpp
	src/sprite_weapon.cpp
	src/sprite_weapon.h
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
	src/std_clock.h
	src/string_view.cpp
	src/string_view.h
	src/statustext_overlay.cpp
	src/statustext_overlay.h
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
	src/translation.cpp
	src/translation.h
	src/util_macro.h
	src/utils.cpp
	src/utils.h
	src/version.cpp
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
	src/window_battlestatus.cpp
	src/window_battlestatus.h
	src/window_command.cpp
	src/window_command.h
	src/window_command_horizontal.cpp
	src/window_command_horizontal.h
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
	src/window_input_settings.cpp
	src/window_input_settings.h
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
	src/window_settings.cpp
	src/window_settings.h
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
	src/external/rang.hpp
)

# These are actually unused when building in CMake
#	src/platform/opendingux/opendingux_input_buttons.cpp
#	src/platform/psp/psp_input_buttons.cpp

target_sources(${PROJECT_NAME} PRIVATE
	src/multiplayer/connection.cpp
	src/multiplayer/connection.h
	src/multiplayer/client_connection.cpp
	src/multiplayer/client_connection.h
	src/multiplayer/server.cpp
	src/multiplayer/server.h
	src/multiplayer/tcp_socket.cpp
	src/multiplayer/tcp_socket.h
	src/multiplayer/game_multiplayer.h
	src/multiplayer/game_playerother.h
	src/multiplayer/messages.h
	src/multiplayer/packet.cpp
	src/multiplayer/packet.h
	src/multiplayer/game_multiplayer.cpp
	src/multiplayer/nametag.cpp
	src/multiplayer/nametag.h
	src/multiplayer/chatui.cpp
	src/multiplayer/chatui.h
	src/multiplayer/playerother.cpp
	src/multiplayer/playerother.h
	src/multiplayer/strfnd.h
)

add_library(${PROJECT_NAME}_server OBJECT
	src/utils.cpp
	src/multiplayer/connection.cpp
	src/multiplayer/tcp_socket.cpp
	src/multiplayer/packet.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optimize floating point math functions into intrinsics and don't check or set errno (i.e. sqrt(-1))
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(${PROJECT_NAME} PUBLIC "-fno-math-errno")
endif()

# Version setup
set(PLAYER_VERSION ${PROJECT_VERSION})
if(NOT PROJECT_VERSION_TWEAK)
	# a kludge to match the valid version format
	set(PROJECT_VERSION_TWEAK 0)
	if(NOT PROJECT_VERSION_PATCH)
		set(PROJECT_VERSION_PATCH 0)
	endif()
endif()
set(PLAYER_VERSION_FULL ${PLAYER_VERSION})
string(CONCAT PROJECT_VERSION
	${PROJECT_VERSION_MAJOR} "." ${PROJECT_VERSION_MINOR} "."
	${PROJECT_VERSION_PATCH} "." ${PROJECT_VERSION_TWEAK})

set(PLAYER_VERSION_GIT "")
git_get_exact_tag(GIT_TAG)
# Do not include a hash, if we are building a release tag
if(NOT GIT_TAG)
	# otherwise concatenate a version with hash
	git_describe(GIT_DESCRIPTION)
	if(GIT_DESCRIPTION)
		string(REPLACE "-" ";" GIT_DESCRIPTION ${GIT_DESCRIPTION})
		list(LENGTH GIT_DESCRIPTION GIT_DESCRIPTION_PARTS)
		set(GIT_MESSAGE "Found git info: ")
		if(GIT_DESCRIPTION_PARTS EQUAL 3)
			list(GET GIT_DESCRIPTION 0 GIT_TAG)
			list(GET GIT_DESCRIPTION 1 GIT_COMMITS)
			list(GET GIT_DESCRIPTION 2 GIT_HASH)
			string(APPEND GIT_MESSAGE "${GIT_COMMITS} commits since tag \"${GIT_TAG}\", ")
			string(PREPEND GIT_COMMITS "+")
		else()
			# no tags found, only hash
			list(GET GIT_DESCRIPTION 0 GIT_HASH)
		endif()
		# strip the g prefix
		string(SUBSTRING ${GIT_HASH} 1 -1 GIT_HASH)
		set(PLAYER_VERSION_GIT "git${GIT_COMMITS}@${GIT_HASH}")
		string(APPEND GIT_MESSAGE "object hash is ${GIT_HASH}")
		git_local_changes(GIT_DIRTY)
		if(GIT_DIRTY STREQUAL "DIRTY")
			string(APPEND PLAYER_VERSION_GIT "-dirty")
			string(APPEND GIT_MESSAGE ", you have uncommitted changes")
		endif()
		string(APPEND PLAYER_VERSION_FULL "-${PLAYER_VERSION_GIT}")

		message(STATUS "${GIT_MESSAGE}")
	endif()
endif()

string(TIMESTAMP PLAYER_DATE "(%Y-%m-%d)")
set(PLAYER_VERSION_APPEND ${PLAYER_DATE} CACHE STRING "Additional version information to include")
set_property(SOURCE src/version.cpp PROPERTY COMPILE_DEFINITIONS
	EP_VERSION="${PLAYER_VERSION}";
	EP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR};
	EP_VERSION_MINOR=${PROJECT_VERSION_MINOR};
	EP_VERSION_PATCH=${PROJECT_VERSION_PATCH};
	EP_VERSION_TWEAK=${PROJECT_VERSION_TWEAK};
	EP_VERSION_APPEND="${PLAYER_VERSION_APPEND}";
	EP_VERSION_GIT="${PLAYER_VERSION_GIT}"
)

# Platform setup
set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "Platform to compile for. Options: SDL2 SDL1 libretro psvita 3ds switch wii amigaos4")
set_property(CACHE PLAYER_TARGET_PLATFORM PROPERTY STRINGS SDL2 SDL1 libretro psvita 3ds switch wii amigaos4)
set(PLAYER_BUILD_EXECUTABLE ON)
set(PLAYER_TEST_LIBRARIES ${PROJECT_NAME})

if(NOT BUILD_CLIENT AND BUILD_SERVER)
	set(PLAYER_TARGET_PLATFORM OFF)
	set(PLAYER_BUILD_EXECUTABLE OFF)
	set(PLAYER_AUDIO_BACKEND OFF)
endif()

if(ANDROID AND PLAYER_GRADLE_BUILD)
	# Build invoked by Gradle
	# Ugly: Gradle has no way to branch based on the ABI
	# Inject correct search path based on PLAYER_ANDROID_TOOLCHAIN_PATH
	foreach(f ${PLAYER_ANDROID_TOOLCHAIN_PATH})
		list(APPEND CMAKE_FIND_ROOT_PATH "${f}/${ANDROID_ABI}-toolchain")
	endforeach(f)
endif()

if(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/audio.cpp
		src/platform/sdl/audio.h
		src/platform/sdl/sdl2_ui.cpp
		src/platform/sdl/sdl2_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=2)

	# SDL2 depends on some systems on SDL2::SDL2main but SDL2::SDL2 is not always a dependency of it
	# Manually add the dependencies
	player_find_package(NAME SDL2
		VERSION 2.0.5
		TARGET SDL2::SDL2
		REQUIRED)

	if(TARGET SDL2::SDL2main)
		target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)
	endif()

	if(ANDROID)
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/builds/android/app/src/gamebrowser)
		set(PLAYER_BUILD_EXECUTABLE OFF)
	endif()
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL1")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/audio.cpp
		src/platform/sdl/audio.h
		src/platform/sdl/axis.h
		src/platform/sdl/sdl_ui.cpp
		src/platform/sdl/sdl_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=1)

	player_find_package(NAME SDL1 TARGET SDL::SDLmain REQUIRED)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "libretro")
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=LibretroUi USE_LIBRETRO=1)
	set(PLAYER_BUILD_EXECUTABLE OFF)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/builds/libretro)
	target_link_libraries(${PROJECT_NAME} retro_common)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "3ds")
	if(NOT NINTENDO_3DS)
		message(FATAL_ERROR "Missing toolchain file! Use '-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake' option.")
	endif()
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=CtrUi PLAYER_NINTENDO)
	target_compile_options(${PROJECT_NAME} PUBLIC -Wno-psabi) # Remove abi warning after devkitarm ships newer gcc
	# generate gfx assets
	enable_language(ASM)
	ctr_add_graphics_target(keyboard IMAGE
		INPUTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/keyboard.png
		OPTIONS -f rgba)
	ctr_add_graphics_target(battery ATLAS
		INPUTS
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/empty.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/1bar.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/2bars.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/3bars.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/4bars.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/full.png
			${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/battery/adapter.png
		OPTIONS -f rgba)
	dkp_add_embedded_binary_library(3ds-assets keyboard battery)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
	target_sources(${PROJECT_NAME} INTERFACE
		$<TARGET_OBJECTS:3ds-assets> PRIVATE
		src/platform/3ds/audio.cpp
		src/platform/3ds/audio.h
		src/platform/3ds/clock.cpp
		src/platform/3ds/clock.h
		src/platform/3ds/input_buttons.cpp
		src/platform/3ds/ui.cpp
		src/platform/3ds/ui.h)
	target_link_libraries(${PROJECT_NAME} 3ds-assets)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "psvita")
	if(NOT VITA)
		message(FATAL_ERROR "Missing toolchain file! Use '-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake' option.")
	endif()
	include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=Psp2Ui)
	target_compile_options(${PROJECT_NAME} PUBLIC -Wno-psabi) # Remove abi warning after vitasdk ships newer gcc
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/psvita/audio.cpp
		src/platform/psvita/audio.h
		src/platform/psvita/clock.cpp
		src/platform/psvita/clock.h
		src/platform/psvita/input_buttons.cpp
		src/platform/psvita/ui.cpp
		src/platform/psvita/ui.h)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "switch")
	if(NOT NINTENDO_SWITCH)
		message(FATAL_ERROR "Missing toolchain file! Use '-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake' option.")
	endif()
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=NxUi PLAYER_NINTENDO)
	find_package(OpenGL CONFIG REQUIRED)
	find_library(GLAD glad REQUIRED)
	target_link_libraries(${PROJECT_NAME} OpenGL::EGL)
	# generate gfx assets
	enable_language(ASM)
	dkp_add_embedded_binary_library(switch-assets "${CMAKE_CURRENT_SOURCE_DIR}/resources/switch/touch_ui.png")
	target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
	target_sources(${PROJECT_NAME} INTERFACE
		$<TARGET_OBJECTS:switch-assets> PRIVATE
		src/platform/switch/audio.cpp
		src/platform/switch/audio.h
		src/platform/switch/clock.cpp
		src/platform/switch/clock.h
		src/platform/switch/input_buttons.cpp
		src/platform/switch/ui.cpp
		src/platform/switch/ui.h)
	target_link_libraries(${PROJECT_NAME} switch-assets)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "wii")
	if(NOT NINTENDO_WII)
		message(FATAL_ERROR "Missing toolchain file! Use '-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Wii.cmake' option.")
	endif()
	find_package(SDL REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=1 PLAYER_NINTENDO)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL_INCLUDE_DIR})
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/wii/clock.cpp
		src/platform/wii/clock.h
		src/platform/wii/input_buttons.cpp
		src/platform/sdl/audio.cpp
		src/platform/sdl/audio.h
		src/platform/sdl/axis.h
		src/platform/sdl/sdl_ui.cpp
		src/platform/sdl/sdl_ui.h)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "amigaos4")
	if(NOT AMIGAOS4)
		message(FATAL_ERROR "Missing toolchain file! Use '-DCMAKE_TOOLCHAIN_FILE=path/to/ppc-amigaos.cmake' option.")
	endif()
	find_package(SDL REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=1)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL_INCLUDE_DIR})
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/audio.cpp
		src/platform/sdl/audio.h
		src/platform/sdl/axis.h
		src/platform/sdl/sdl_ui.cpp
		src/platform/sdl/sdl_ui.h)
	message(FATAL_ERROR "Invalid target platform")
endif()

# Shared by homebrew platforms
if(${PLAYER_TARGET_PLATFORM} MATCHES "^(3ds|psvita|switch|wii)$")
	target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions -fno-rtti)
	set(CMAKE_DL_LIBS "") # hack4icu!
	set(PLAYER_ENABLE_TESTS OFF)
	option(PLAYER_VERSIONED_PACKAGES "Create zip packages with versioned name (for internal use)" ON)
endif()
# Make romfs available
if(${PLAYER_TARGET_PLATFORM} MATCHES "^(3ds|switch)$")
	option(PLAYER_ROMFS "Embedd a directory in the executable" OFF)
	set(PLAYER_ROMFS_PATH "romfs" CACHE PATH "Directory to include in executable as romfs:/ path")
	set(ROMFS_ARG "NO_ROMFS_IGNORE_ME")
	if(PLAYER_ROMFS)
		set(ROMFS_ARG "ROMFS")
	endif()
endif()

# Amiga
if(AMIGA)
	target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions -fno-rtti)
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_AMIGA)
	if(MORPHOS)
		target_compile_definitions(${PROJECT_NAME} PUBLIC
			_GLIBCXX_USE_C99 _NO_PPCINLINE __MORPHOS_SHAREDLIBS)
		# -noixemul handled as "M68K_CRT" in toolchain file
		target_sources(${PROJECT_NAME} PRIVATE
			src/platform/morphos/integration.cpp)
		set_property(SOURCE src/platform/morphos/integration.cpp
			PROPERTY COMPILE_DEFINITIONS PLAYER_VERSION="${PLAYER_VERSION_FULL}"
		)
	endif()
	if(AMIGAOS4)
		# FIXME: needed?
		target_compile_definitions(${PROJECT_NAME} PUBLIC
			_GLIBCXX_USE_C99_STDINT_TR1 _GLIBCXX_USE_C99_STDIO)
		target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/src/platform/amigaos4)
	endif()
	if(AROS)
		message(WARNING "AROS is unsupported currently. Patches welcome!")
	endif()
endif()

if(NOT PLAYER_BUILD_EXECUTABLE AND BUILD_SHARED_LIBS)
	# Need fPIC when compiling a shared library (e.g. libretro.so)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(ANDROID)
	target_link_libraries(${PROJECT_NAME} android log)
endif()

if(WIN32)
	target_sources(${PROJECT_NAME} PRIVATE
		src/registry.cpp
		src/platform/windows/utils.cpp
		src/platform/windows/utils.h
		src/platform/windows/midiout_device_win32.cpp
		src/platform/windows/midiout_device_win32.h)
endif()

if(APPLE)
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/macos/utils.mm
		src/platform/macos/utils.h
		src/platform/macos/midiout_device_coreaudio.cpp
		src/platform/macos/midiout_device_coreaudio.h)

	find_library(MACOSFOUNDATION Foundation)
	find_Library(MACOSAUDIOUNIT AudioUnit)
	find_library(MACOSAUDIOTOOLBOX AudioToolbox)
	target_link_libraries(${PROJECT_NAME} ${MACOSFOUNDATION} ${MACOSAUDIOUNIT} ${MACOSAUDIOTOOLBOX})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	option(PLAYER_JS_BUILD_SHELL "Build the Player executable as a shell file (.html) instead of a standalone javascript file (.js)" OFF)
	set(PLAYER_JS_GAME_URL "games/" CACHE STRING "Game URL/directory where the web player searches for games")
	set(PLAYER_JS_OUTPUT_NAME "easyrpg-player" CACHE STRING "Output name of the js, html and wasm files")
	set_property(SOURCE src/async_handler.cpp APPEND PROPERTY COMPILE_DEFINITIONS "EM_GAME_URL=\"${PLAYER_JS_GAME_URL}\"")
	target_sources(${PROJECT_NAME} PRIVATE src/external/picojson.h)
endif()

# Endianess check
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
	if (CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
		target_compile_definitions(${PROJECT_NAME} PRIVATE WORDS_BIGENDIAN=1)
	endif()
else()
	include(TestBigEndian)
	test_big_endian(WORDS_BIGENDIAN)
	if(WORDS_BIGENDIAN)
		target_compile_definitions(${PROJECT_NAME} PRIVATE WORDS_BIGENDIAN=1)
	endif()
endif()

# App Icon (for macOS)
set(${PROJECT_NAME}_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Player.icns")

# Do not code sign on Macs
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.easyrpg.player")

# liblcf
option(PLAYER_BUILD_LIBLCF "Instead of detecting liblcf, clone liblcf into lib/liblcf and build it along with the Player. This is convenient for development." OFF)
set(PLAYER_BUILD_LIBLCF_GIT "https://github.com/EasyRPG/liblcf.git" CACHE STRING "Git repository of liblcf to clone. Requires PLAYER_BUILD_LIBLCF=ON.")
set(PLAYER_BUILD_LIBLCF_BRANCH "master" CACHE STRING "Branch of the liblcf repository to clone. Requires PLAYER_BUILD_LIBLCF=ON.")

# build libs
option(PLAYER_BUILD_LIBS "Clone all libs into lib/ and built it along with the Player." OFF)
if(PLAYER_BUILD_LIBS)
	set(PLAYER_BUILD_LIBLCF ON)
endif()

if(PLAYER_BUILD_LIBLCF)
	# liblcf is built as part of this cmake file
	set(LIBLCF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf")
	if(NOT EXISTS ${LIBLCF_PATH})
		find_package(Git REQUIRED)
		execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1" "--branch"
			"${PLAYER_BUILD_LIBLCF_BRANCH}"
			"${PLAYER_BUILD_LIBLCF_GIT}"
			"${LIBLCF_PATH}")
	endif()
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules")

	# Always build static liblcf and do not install it
	function(add_liblcf)
		set(BUILD_SHARED_LIBS OFF)
		set(LIBLCF_ENABLE_INSTALL OFF)
		add_subdirectory(${LIBLCF_PATH})
	endfunction()
	add_liblcf()
	target_link_libraries(${PROJECT_NAME} lcf)
else()
	# Use system package
	player_find_package(NAME liblcf VERSION 0.8
		TARGET liblcf::liblcf
		REQUIRED)
endif()

if(PLAYER_BUILD_LIBS)
	set(SOCKPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/sockpp")
	if(NOT EXISTS ${SOCKPP_PATH})
		find_package(Git REQUIRED)
		execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1" "--branch"
			"v0.8.1"
			"https://github.com/fpagliughi/sockpp"
			"${SOCKPP_PATH}")
		include(Patch)
		execute_process(COMMAND ${PATCH_EXECUTABLE} -p1 -d "${SOCKPP_PATH}"
			--input=${CMAKE_CURRENT_SOURCE_DIR}/patches/sockpp.patch )
	endif()
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	set(SOCKPP_BUILD_SHARED OFF)
	set(SOCKPP_BUILD_STATIC ON)
	add_subdirectory(${SOCKPP_PATH})
	target_link_libraries(${PROJECT_NAME} sockpp-static)
endif()

if(BUILD_CLIENT) # BUILD_CLIENT

# Detect all required libraries
player_find_package(NAME PNG TARGET PNG::PNG REQUIRED)
player_find_package(NAME fmt TARGET fmt::fmt REQUIRED)

# Do not use player_find_package. enable_language used by pixman on Android does work properly inside function calls
find_package(Pixman REQUIRED)
target_link_libraries(${PROJECT_NAME} PIXMAN::PIXMAN)

# Always enable Wine registry support on non-Windows, but not for console ports
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT ${PLAYER_TARGET_PLATFORM} MATCHES "^(psvita|3ds|switch|wii)$")
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_WINE=1)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_HARFBUZZ "Enable HarfBuzz text shaping (Requires FreeType)" ON "PLAYER_WITH_FREETYPE" OFF)

# Freetype is not really broken however only using FT config without HB config causes problems
player_find_package(NAME freetype
	CONDITION PLAYER_WITH_FREETYPE
	DEFINITION HAVE_FREETYPE
	TARGET freetype
	CONFIG_BROKEN)

if(TARGET freetype)
	# Harfbuzz config is broken (see https://github.com/harfbuzz/harfbuzz/issues/2316)
	# They do not plan to fix this as they are moving to meson
	player_find_package(NAME harfbuzz
		CONDITION PLAYER_WITH_HARFBUZZ
		DEFINITION HAVE_HARFBUZZ
		TARGET harfbuzz::harfbuzz
		CONFIG_BROKEN)
endif()

# Sound system to use
if(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2")
	set(PLAYER_AUDIO_BACKEND "SDL2" CACHE STRING "Audio system to use. Options: SDL2 OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL2 OFF)

	if(${PLAYER_AUDIO_BACKEND} STREQUAL "SDL2_mixer")
		message(FATAL_ERROR "SDL2_mixer is not supported anymore. Use SDL2 instead.")
	endif()
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL1")
	set(PLAYER_AUDIO_BACKEND "SDL1" CACHE STRING "Audio system to use. Options: SDL1 OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL1 OFF)
else()
	set(PLAYER_AUDIO_BACKEND "Default" CACHE STRING "Audio system to use. Options: Default OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS Default OFF)

	if(${PLAYER_AUDIO_BACKEND} STREQUAL "Default")
		# Assuming that all platforms not targeting SDL have only one audio backend
		set(PLAYER_AUDIO_BACKEND ${PLAYER_TARGET_PLATFORM})
	endif()
endif()

# Configure Audio backends
if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL2|SDL1|libretro|psvita|3ds|switch|wii|amigaos4)$")
	set(PLAYER_HAS_AUDIO ON)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	if(${PLAYER_AUDIO_BACKEND} STREQUAL "libretro")
		if (WIN32 OR UNIX OR APPLE)
			set(SUPPORT_NATIVE_MIDI ON)
		endif()
	endif()

	if(WIN32 OR APPLE)
		set(SUPPORT_NATIVE_MIDI ON)
	elseif(UNIX)
		if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12)
			find_package(ALSA)
			if(ALSA_FOUND)
				set(SUPPORT_NATIVE_MIDI ON)
			endif()
		endif()
	endif()

	CMAKE_DEPENDENT_OPTION(PLAYER_WITH_NATIVE_MIDI "Play MIDI audio using the API of the operating system" ON "SUPPORT_NATIVE_MIDI" OFF)
	if(PLAYER_WITH_NATIVE_MIDI)
		target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_NATIVE_MIDI=1)
		if(WIN32)
			target_link_libraries(${PROJECT_NAME} "winmm")
		elseif(ALSA_FOUND)
			target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_ALSA=1)
			target_sources(${PROJECT_NAME} PRIVATE
				src/platform/linux/midiout_device_alsa.cpp
				src/platform/linux/midiout_device_alsa.h
			)
			target_link_libraries(${PROJECT_NAME} ALSA::ALSA)
			find_package(Threads)
			if(Threads_FOUND)
				target_link_libraries(${PROJECT_NAME} Threads::Threads)
			endif()
		endif()
	endif()

	# Provide fmmidi options
	option(PLAYER_ENABLE_FMMIDI "Enable internal MIDI sequencer. Will be used when external MIDI library fails." ON)
	if(PLAYER_ENABLE_FMMIDI)
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_FMMIDI=1)
	endif()
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
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_FLUIDSYNTH "Play MIDI audio with fluidsynth" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_FLUIDLITE "Play MIDI audio with fluidlite" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_WITH_XMP "Play MOD audio with libxmp" ON "PLAYER_HAS_AUDIO" OFF)
CMAKE_DEPENDENT_OPTION(PLAYER_ENABLE_DRWAV "Play WAV audio with dr_wav (built-in). Unsupported files are played by libsndfile." ON "PLAYER_HAS_AUDIO" OFF)

if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL2|SDL1|libretro|psvita|3ds|switch|wii|amigaos4)$")
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
		if(NOT TARGET speexdsp::speexdsp)
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
		DEFINITION HAVE_LIBMPG123
		TARGET MPG123::libmpg123)

	# libsndfile
	player_find_package(NAME SndFile
		CONDITION PLAYER_WITH_LIBSNDFILE
		DEFINITION HAVE_LIBSNDFILE
		TARGET SndFile::sndfile)

	# libvorbis
	player_find_package(NAME Vorbis
		CONDITION PLAYER_WITH_OGGVORBIS
		DEFINITION HAVE_OGGVORBIS
		TARGET Vorbis::vorbisfile)

	if(NOT TARGET Vorbis::vorbisfile)
		player_find_package(NAME Tremor
			CONDITION PLAYER_WITH_OGGVORBIS
			DEFINITION HAVE_TREMOR
			TARGET Tremor::Tremor)
	endif()

	# opusfile
	player_find_package(NAME OpusFile
		CONDITION PLAYER_WITH_OPUS
		DEFINITION HAVE_OPUS
		TARGET OpusFile::opusfile)

	# wildmidi
	player_find_package(NAME WildMidi
		CONDITION PLAYER_WITH_WILDMIDI
		DEFINITION HAVE_LIBWILDMIDI
		TARGET "WildMidi::libwildmidi;WildMidi::libwildmidi-static")

	# fluidsynth
	player_find_package(NAME FluidSynth
		CONDITION PLAYER_WITH_FLUIDSYNTH
		DEFINITION HAVE_FLUIDSYNTH
		TARGET FluidSynth::libfluidsynth)

	# fluidlite
	if(NOT TARGET FluidSynth::libfluidsynth)
		player_find_package(NAME FluidLite
			CONDITION PLAYER_WITH_FLUIDLITE
			DEFINITION HAVE_FLUIDLITE
			TARGET FluidLite::fluidlite)
	endif()

	# xmp (lite)
	player_find_package(NAME XMP
		VERSION 4.5.0
		CONDITION PLAYER_WITH_XMP
		DEFINITION HAVE_LIBXMP
		TARGET XMP::XMP)

	if(PLAYER_ENABLE_FMMIDI)
		target_sources(${PROJECT_NAME} PRIVATE
			src/decoder_fmmidi.cpp
			src/decoder_fmmidi.h
			src/midiprogram.h
			src/midisynth.cpp
			src/midisynth.h)
	endif()

	if(PLAYER_ENABLE_DRWAV)
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_DRWAV=1)
		target_sources(${PROJECT_NAME} PRIVATE
			src/decoder_drwav.cpp
			src/decoder_drwav.h
			src/external/dr_wav.h)
	endif()
endif()

# Executable
if(${PLAYER_BUILD_EXECUTABLE} AND (${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2" OR ${PLAYER_TARGET_PLATFORM} STREQUAL "SDL1"))
	if(APPLE)
		set(EXE_NAME "EasyRPG-Player.app")
		set_source_files_properties(${${PROJECT_NAME}_BUNDLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
		add_executable(${EXE_NAME} MACOSX_BUNDLE "src/platform/sdl/main.cpp" ${${PROJECT_NAME}_BUNDLE_ICON})
		configure_file(resources/macos/Info.plist.in resources/macos/Info.plist @ONLY)
		set_target_properties(${EXE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/resources/macos/Info.plist")
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "EasyRPG Player")
	else()
		set(EXE_NAME ${PROJECT_NAME}_exe)
		if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
			add_executable(${EXE_NAME} "src/platform/emscripten/main.cpp")
		else()
			add_executable(${EXE_NAME} "src/platform/sdl/main.cpp")
		endif()
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "easyrpg-player")
	endif()

	if(WIN32)
		# Open console for Debug builds
		set_target_properties(${EXE_NAME} PROPERTIES WIN32_EXECUTABLE $<$<NOT:$<CONFIG:Debug>>:TRUE>)

		# Add resources
		string(REPLACE "." "," RC_VERSION ${PROJECT_VERSION})
		string(TIMESTAMP RC_YEAR "%Y")
		configure_file(resources/windows/player.rc.in resources/windows/player.rc @ONLY)
		target_sources(${EXE_NAME} PRIVATE
			resources/windows/player.manifest
			"${CMAKE_CURRENT_BINARY_DIR}/resources/windows/player.rc")
		if(MSVC)
			target_link_options(${EXE_NAME} PUBLIC /DEBUG:FULL)
			message(STATUS "x64dbg: Add x64dbg source file viewing support")
		endif()

		# Set Visual Studio startup project
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})

		# Change executable name
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "Player")
	endif()

	target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		# Do not confuse this with our own -DUSE_SDL
		# Prevent Emscripten from pulling in bundled SDL headers that break the build
		target_compile_options(${PROJECT_NAME} PUBLIC "-sUSE_SDL=0")

		set(PLAYER_JS_PREJS "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-pre.js")
		set(PLAYER_JS_POSTJS "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-post.js")
		set(PLAYER_JS_SHELL "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-shell.html")

		set_property(TARGET ${EXE_NAME} PROPERTY LINK_FLAGS "-s ALLOW_MEMORY_GROWTH -s EXIT_RUNTIME -s ASYNCIFY -s ASYNCIFY_IGNORE_INDIRECT -s MINIFY_HTML=0 --pre-js ${PLAYER_JS_PREJS} --post-js ${PLAYER_JS_POSTJS} -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=['$autoResumeAudioContext','$dynCall']")
		set_source_files_properties("src/platform/sdl/main.cpp" PROPERTIES
			OBJECT_DEPENDS "${PLAYER_JS_PREJS};${PLAYER_JS_POSTJS};${PLAYER_JS_SHELL}")

		if(PLAYER_JS_BUILD_SHELL)
			set_target_properties(${EXE_NAME} PROPERTIES SUFFIX ".html")
			set_property(TARGET ${EXE_NAME} APPEND_STRING PROPERTY LINK_FLAGS " --shell-file ${PLAYER_JS_SHELL}")
		endif()

		target_link_libraries(${EXE_NAME} "idbfs.js")

		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "${PLAYER_JS_OUTPUT_NAME}")
	endif()

	# installation
	include(GNUInstallDirs)
	if(APPLE)
		install(TARGETS ${EXE_NAME} RUNTIME DESTINATION BUNDLE DESTINATION "${CMAKE_BINARY_DIR}/Package")
	else()
		install(TARGETS ${EXE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()

	if(UNIX)
		# Install desktop entry, icons and metainfo
		install(FILES resources/unix/easyrpg-player.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
		install(FILES resources/unix/icon-48.png RENAME easyrpg-player.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps) # legacy
		install(FILES resources/unix/icon-48.png RENAME easyrpg-player.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)
		install(FILES resources/unix/easyrpg-player.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
		install(FILES resources/unix/easyrpg-player.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	endif()

	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:${EXE_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		# Emscripten does not install the wasm file (or the js file when building a shell)
		if(PLAYER_JS_BUILD_SHELL)
			install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLAYER_JS_OUTPUT_NAME}.js DESTINATION ${CMAKE_INSTALL_BINDIR})
		endif()
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLAYER_JS_OUTPUT_NAME}.wasm DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()
elseif(${PLAYER_TARGET_PLATFORM} MATCHES "^(psvita|3ds|switch|wii)$")
	if(NINTENDO_3DS)
		add_executable(easyrpg-player src/platform/3ds/main.cpp)
		target_link_libraries(easyrpg-player
			${PROJECT_NAME}
			-lcitro2d
			-lcitro3d)
		ctr_generate_smdh(easyrpg-player.smdh
			NAME "EasyRPG Player ${PLAYER_VERSION_FULL}"
			DESCRIPTION "Play your RPG Maker 2000/2003 games everywhere"
			AUTHOR "EasyRPG Team"
			ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/3ds/icon.png)
		ctr_create_3dsx(easyrpg-player
			SMDH easyrpg-player.smdh
			${ROMFS_ARG} ${PLAYER_ROMFS_PATH})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.3dsx
				resources/3ds/easyrpg-player.xml # consider removing, *hax² is mostly obsolete
			DESTINATION easyrpg-player COMPONENT 3ds)
	elseif(NINTENDO_SWITCH)
		add_executable(easyrpg-player src/platform/switch/main.cpp)
		target_link_libraries(easyrpg-player
			${PROJECT_NAME}
			${GLAD})
		nx_generate_nacp(easyrpg-player.nacp
			NAME "EasyRPG Player"
			AUTHOR "EasyRPG Team"
			VERSION "${PLAYER_VERSION_FULL}")
		nx_create_nro(easyrpg-player
			NACP easyrpg-player.nacp
			ICON "${PROJECT_SOURCE_DIR}/resources/switch/icon.jpg"
			${ROMFS_ARG} ${PLAYER_ROMFS_PATH})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.nro
			DESTINATION easyrpg-player COMPONENT switch)
	elseif(NINTENDO_WII)
		add_executable(easyrpg-player
			src/platform/wii/main.h
			src/platform/wii/main.cpp)
		target_link_libraries(easyrpg-player
			${PROJECT_NAME}
			${SDL_LIBRARIES}
			-laesnd
			-lfat
			-lwiikeyboard)
		ogc_create_dol(easyrpg-player)
		string(TIMESTAMP WII_DATE "%Y%m%d000000")
		configure_file(resources/wii/meta.xml.in resources/wii/meta.xml @ONLY)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.dol
			RENAME boot.dol
			DESTINATION easyrpg-player COMPONENT wii)
		install(FILES resources/wii/icon.png
			${CMAKE_CURRENT_BINARY_DIR}/resources/wii/meta.xml
			DESTINATION easyrpg-player COMPONENT wii)
	elseif(VITA)
		set(CMAKE_EXECUTABLE_SUFFIX .elf)
		add_executable(easyrpg-player src/platform/psvita/main.cpp)
		target_link_libraries(easyrpg-player
			${PROJECT_NAME}
			vita2d
			-Wl,--whole-archive pthread -Wl,--no-whole-archive
			SceLibKernel_stub
			SceDisplay_stub
			SceGxm_stub
			SceSysmodule_stub
			SceCtrl_stub
			SceTouch_stub
			ScePgf_stub
			ScePower_stub
			SceCommonDialog_stub
			SceAudio_stub
			SceAppMgr_stub)
		vita_create_self(eboot.bin easyrpg-player.elf $<$<NOT:$<CONFIG:Debug>>:STRIPPED>)
		set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")
		configure_file(resources/psvita/template.xml.in resources/psvita/template.xml @ONLY)
		vita_create_vpk(EasyRPG-Player.vpk "EASYRPG01" eboot.bin
			VERSION "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK}"
			NAME "EasyRPG Player"
			FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/psvita/icon0.png" sce_sys/icon0.png
			FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/psvita/bg.png" sce_sys/livearea/contents/bg.png
			FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/psvita/startup.png" sce_sys/livearea/contents/startup.png
			FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/psvita/template.xml" sce_sys/livearea/contents/template.xml
			FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/psvita/touch_ui.png" touch_ui.png)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/EasyRPG-Player.vpk
			DESTINATION . COMPONENT psvita)
	endif()
	install(TARGETS easyrpg-player RUNTIME DESTINATION . COMPONENT debug)
	# debug information
	if(${PLAYER_TARGET_PLATFORM} MATCHES "^(3ds|switch|wii)$")
		dkp_target_generate_symbol_list(easyrpg-player)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.map
				${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.lst
			DESTINATION . COMPONENT debug)
	endif()
	# packaging for distribution
	set(CPACK_GENERATOR "ZIP")
	set(CPACK_SOURCE_GENERATOR ";") # disable source archive
	set(CPACK_ARCHIVE_COMPONENT_INSTALL ON) # split archives
	set(CPACK_PACKAGE_FILE_NAME "easyrpg-player")
	if(PLAYER_VERSIONED_PACKAGES)
		string(APPEND CPACK_PACKAGE_FILE_NAME "-${PLAYER_VERSION}")
	endif()
	set(CPACK_ARCHIVE_DEBUG_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${PLAYER_TARGET_PLATFORM}-debug")
	set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # we do this manually
	include(CPack)
elseif(${PLAYER_TARGET_PLATFORM} STREQUAL "amigaos4")
	add_executable(easyrpg-player "src/platform/sdl/main.cpp")
	target_link_libraries(easyrpg-player
		${PROJECT_NAME}
		${SDL_LIBRARIES})
	# FIXME: packaging?
else()
	if(${PLAYER_TARGET_PLATFORM} STREQUAL "libretro")
		add_library(easyrpg_libretro
			src/platform/libretro/audio.cpp
			src/platform/libretro/audio.h
			src/platform/libretro/clock.cpp
			src/platform/libretro/clock.h
			src/platform/libretro/input_buttons.cpp
			src/platform/libretro/ui.cpp
			src/platform/libretro/ui.h
			src/platform/libretro/midiout_device.cpp
			src/platform/libretro/midiout_device.h
		)
		set_property(SOURCE src/platform/libretro/ui.cpp
			PROPERTY COMPILE_DEFINITIONS PLAYER_VERSION_FULL="${PLAYER_VERSION_FULL}"
		)
		target_link_libraries(easyrpg_libretro ${PROJECT_NAME})
		set_target_properties(easyrpg_libretro PROPERTIES PREFIX "")
		set(PLAYER_TEST_LIBRARIES "easyrpg_libretro")

		if(WIN32)
			set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT easyrpg_libretro)
		endif()
	elseif(ANDROID AND ${PLAYER_TARGET_PLATFORM} STREQUAL "SDL2")
		add_library(easyrpg_android
			src/platform/android/android.h
			src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.cpp
			src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.h
			src/platform/android/filesystem_apk.cpp
			src/platform/android/filesystem_apk.h
			src/platform/android/filesystem_saf.cpp
			src/platform/android/filesystem_saf.h
			src/platform/sdl/main.cpp)
		target_link_libraries(easyrpg_android ${PROJECT_NAME})
		set_target_properties(easyrpg_android PROPERTIES DEBUG_POSTFIX "")
		set(PLAYER_TEST_LIBRARIES "easyrpg_android")
	else()
		message(FATAL_ERROR "Unsupported library target platform ${PLAYER_TARGET_PLATFORM}")
	endif()
endif()

if (NOT BUILD_SERVER)
	set_target_properties(${PROJECT_NAME}_server PROPERTIES EXCLUDE_FROM_ALL ON)
endif()

endif() # BUILD_CLIENT

if(BUILD_SERVER)
	if (NOT BUILD_CLIENT)
		set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
	endif()

	player_find_package(NAME fmt TARGET fmt::fmt REQUIRED)
	target_link_libraries(${PROJECT_NAME}_server lcf)
	target_link_libraries(${PROJECT_NAME}_server sockpp-static)

	add_executable(${PROJECT_NAME}_server_exe "src/multiplayer/server.cpp")
	set_target_properties(${PROJECT_NAME}_server_exe PROPERTIES OUTPUT_NAME "easyrpg-player-server")
	target_link_libraries(${PROJECT_NAME}_server_exe ${PROJECT_NAME}_server)
	install(TARGETS ${PROJECT_NAME}_server_exe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

	target_compile_definitions(${PROJECT_NAME}_server PRIVATE SERVER)
	target_compile_definitions(${PROJECT_NAME}_server_exe PRIVATE SERVER)
endif()

# manpage
# Handled as extra target "man"
set(MAN_NAME easyrpg-player.6)
find_program(ASCIIDOCTOR_EXECUTABLE asciidoctor)
set(MANUAL_STATUS "Unavailable")
if(ASCIIDOCTOR_EXECUTABLE)
	add_custom_command(OUTPUT resources/unix/${MAN_NAME}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/resources/unix
		COMMAND ${ASCIIDOCTOR_EXECUTABLE} -a player_version="${PLAYER_VERSION}"
			-b manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources/unix
			${CMAKE_CURRENT_SOURCE_DIR}/resources/unix/${MAN_NAME}.adoc
		DEPENDS resources/unix/${MAN_NAME}.adoc
		COMMENT "(Re-)building manpage ${MAN_NAME}"
		VERBATIM)
	if(UNIX)
		add_custom_target(player_man ALL DEPENDS resources/unix/${MAN_NAME})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/unix/${MAN_NAME}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
	else()
		add_custom_target(player_man DEPENDS resources/unix/${MAN_NAME})
	endif()
	if(NOT TARGET man)
		add_custom_target(man)
	endif()
	add_dependencies(man player_man)
	set(MANUAL_STATUS "Generated")
else()
	# no asciidoctor, distribution archive?
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/unix/${MAN_NAME})
		install(FILES resources/unix/${MAN_NAME} DESTINATION share/man/man6)
		set(MANUAL_STATUS "Bundled")
	endif()
endif()

# bash completion
set(BASHCOMPLETION_STATUS "Unavailable")
find_package(bash-completion QUIET)
if(BASH_COMPLETION_FOUND OR (UNIX AND NOT APPLE))
	set(BASHCOMPLETION_STATUS "Available")
	install(FILES resources/unix/bash-completion/easyrpg-player DESTINATION "${CMAKE_INSTALL_DATADIR}/bash-completion/completions")
endif()

# Doxygen
# Handled as extra target "doc"
find_package(Doxygen)
set(DOXYGEN_STATUS "Unavailable")
if(DOXYGEN_FOUND)
	set(DOXYGEN_STATUS "Available (target \"doc\")")
	# fake autotools variables
	set(PACKAGE_VERSION ${PLAYER_VERSION})
	set(DX_DOCDIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
	set(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
	configure_file(resources/Doxyfile.in resources/Doxyfile @ONLY)
	unset(PACKAGE_VERSION)
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
	file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
	add_executable(test_runner_player EXCLUDE_FROM_ALL ${TEST_FILES})
	set_target_properties(test_runner_player PROPERTIES
		OUTPUT_NAME "test_runner"
		WIN32_EXECUTABLE FALSE
	)

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		target_compile_definitions(test_runner_player PUBLIC EP_NATIVE_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/assets\")
		target_compile_definitions(test_runner_player PUBLIC EP_TEST_PATH=\"/assets\")
		target_link_libraries(test_runner_player "nodefs.js")
	else()
		target_compile_definitions(test_runner_player PUBLIC EP_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/assets\")
	endif()
	target_link_libraries(test_runner_player ${PLAYER_TEST_LIBRARIES})
	if(CMAKE_COLOR_DIAGNOSTICS)
		set(DOCTEST_COLOROPT "--force-colors")
	endif()
	add_custom_target(check_player COMMAND test_runner_player ${DOCTEST_COLOROPT})
	if(NOT TARGET check)
		add_custom_target(check)
	endif()
	add_dependencies(check_player test_runner_player)
	add_dependencies(check check_player)

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		set_property(TARGET test_runner_player APPEND_STRING PROPERTY LINK_FLAGS "-s ALLOW_MEMORY_GROWTH")
	endif()
endif()

# Instrumentation framework
set(PLAYER_ENABLE_INSTRUMENTATION "OFF" CACHE STRING "Build performance instrumentation hooks")
set_property(CACHE PLAYER_ENABLE_INSTRUMENTATION PROPERTY STRINGS OFF VTune)
if (NOT ${PLAYER_ENABLE_INSTRUMENTATION} STREQUAL "OFF")
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_INSTRUMENTATION=${PLAYER_ENABLE_INSTRUMENTATION})
	if (${PLAYER_ENABLE_INSTRUMENTATION} STREQUAL "VTune")
		player_find_package(NAME VTune TARGET VTune::ITT REQUIRED)
	endif()
endif()

# Benchmarks
option(PLAYER_ENABLE_BENCHMARKS "Build benchmarks" OFF)

if(PLAYER_ENABLE_BENCHMARKS)
	find_package(benchmark REQUIRED)
	file(GLOB BENCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bench/*.cpp)
	foreach(i ${BENCH_FILES})
		get_filename_component(name "${i}" NAME_WE)
		add_executable(bench_${name} ${i})
		set_target_properties(bench_${name} PROPERTIES WIN32_EXECUTABLE FALSE)
		target_link_libraries(bench_${name} ${PROJECT_NAME})
		target_link_libraries(bench_${name} benchmark)
	endforeach()
endif()

# Print summary
message(STATUS "")
message(STATUS "Target system: ${PLAYER_TARGET_PLATFORM}")
if(PLAYER_ROMFS)
	message(STATUS "RomFS: Embedding directory \"${PLAYER_ROMFS_PATH}\"")
endif()
message(STATUS "")

if(PLAYER_BUILD_LIBLCF)
	message(STATUS "liblcf: Built from ${PLAYER_BUILD_LIBLCF_GIT}")
	message(STATUS "")
endif()

message(STATUS "Audio backend: ${PLAYER_AUDIO_BACKEND}")
if(${PLAYER_AUDIO_BACKEND} MATCHES "^(SDL2|SDL1|libretro|psvita|3ds|switch|wii)$")
	message(STATUS "")

	set(WAV_LIBS)
	if(PLAYER_ENABLE_DRWAV)
		list(APPEND WAV_LIBS "built-in (dr_wav)")
	endif()
	if(TARGET SndFile::sndfile)
		list(APPEND WAV_LIBS "libsndfile")
	endif()

	if(WAV_LIBS)
		message(STATUS "WAV playback: ${WAV_LIBS}")
	else()
		message(STATUS "WAV playback: None")
	endif()

	set(MIDI_LIBS)
	if(TARGET FluidSynth::libfluidsynth)
		list(APPEND MIDI_LIBS "FluidSynth")
	endif()
	if(TARGET FluidLite::fluidlite)
		list(APPEND MIDI_LIBS "FluidLite")
	endif()
	if(TARGET WildMidi::libwildmidi OR TARGET WildMidi::libwildmidi-static)
		list(APPEND MIDI_LIBS "WildMidi")
	endif()
	if(PLAYER_WITH_NATIVE_MIDI)
		list(APPEND MIDI_LIBS "Native")
	endif()
	if(PLAYER_ENABLE_FMMIDI)
		list(APPEND MIDI_LIBS "built-in (FmMidi)")
	endif()

	if(MIDI_LIBS)
		message(STATUS "MIDI playback: ${MIDI_LIBS}")
	else()
		message(STATUS "MIDI playback: None")
	endif()

	if(TARGET MPG123::libmpg123)
		message(STATUS "MP3 playback: mpg123")
	else()
		message(STATUS "MP3 playback: None")
	endif()

	if(TARGET Vorbis::vorbisfile)
		message(STATUS "Ogg Vorbis playback: libvorbis")
	elseif(TARGET Tremor::Tremor)
		message(STATUS "Ogg Vorbis playback: tremor")
	else()
		message(STATUS "Ogg Vorbis playback: None")
	endif()

	if(TARGET XMP::XMP)
		message(STATUS "MOD playback: libxmp")
	else()
		message(STATUS "MOD playback: None")
	endif()

	if(TARGET OpusFile::opusfile)
		message(STATUS "Opus playback: opusfile")
	else()
		message(STATUS "Opus playback: None")
	endif()

	if(TARGET speexdsp::speexdsp)
		message(STATUS "Resampler: speexdsp")
	elseif(TARGET Samplerate::Samplerate)
		message(STATUS "Resampler: libsamplerate")
	else()
		message(STATUS "Resampler: None")
	endif()
endif()

message(STATUS "")

if(TARGET harfbuzz::harfbuzz)
	message(STATUS "Font rendering: Freetype with Harfbuzz / built-in")
elseif(TARGET freetype)
	message(STATUS "Font rendering: Freetype and built-in")
else()
	message(STATUS "Font rendering: built-in")
endif()

message(STATUS "")

message(STATUS "Manual page: ${MANUAL_STATUS}")
message(STATUS "Bash completion: ${BASHCOMPLETION_STATUS}")
message(STATUS "Doxygen: ${DOXYGEN_STATUS}")

message(STATUS "")

if (${PLAYER_TARGET_PLATFORM} STREQUAL "SDL1")
	message(WARNING "SDL1 is deprecated!")
	message(WARNING "Please migrate to SDL2!")
endif()
