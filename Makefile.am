ACLOCAL_AMFLAGS = --install -I builds/autoconf/m4
include builds/autoconf/aminclude/doxygen.am

EXTRA_DIST = AUTHORS.md README.md builds lib/README.md resources CMakeLists.txt
MOSTLYCLEANFILES = $(DX_CLEANFILES)

bin_PROGRAMS = easyrpg-player

noinst_LTLIBRARIES = libeasyrpg-player.la
libeasyrpg_player_la_SOURCES = \
	src/async_handler.cpp \
	src/async_handler.h \
	src/audio_al.cpp \
	src/audio_al.h \
	src/audio_sdl_mixer.cpp \
	src/audio_sdl_mixer.h \
	src/audio_sdl.cpp \
	src/audio_sdl.h \
	src/audio.cpp \
	src/audio.h \
	src/audio_decoder.cpp \
	src/audio_decoder.h \
	src/audio_generic.cpp \
	src/audio_generic.h \
	src/audio_resampler.cpp \
	src/audio_resampler.h \
	src/audio_secache.cpp \
	src/audio_secache.h \
	src/background.cpp \
	src/background.h \
	src/baseui.cpp \
	src/baseui.h \
	src/battle_animation.cpp \
	src/battle_animation.h \
	src/bitmap.cpp \
	src/bitmap.h \
	src/bitmap_hslrgb.h \
	src/cache.cpp \
	src/cache.h \
	src/color.cpp \
	src/color.h \
	src/decoder_libsndfile.cpp \
	src/decoder_libsndfile.h \
	src/decoder_mpg123.cpp \
	src/decoder_mpg123.h \
	src/decoder_oggvorbis.cpp \
	src/decoder_oggvorbis.h \
	src/decoder_opus.cpp \
	src/decoder_opus.h \
	src/decoder_fmmidi.cpp \
	src/decoder_fmmidi.h \
	src/decoder_wav.cpp \
	src/decoder_wav.h \
	src/decoder_wildmidi.cpp \
	src/decoder_wildmidi.h \
	src/decoder_xmp.cpp \
	src/decoder_xmp.h \
	src/default_graphics.h \
	src/dirent_win.h \
	src/docmain.h \
	src/drawable.h \
	src/exfont.h \
	src/filefinder.cpp \
	src/filefinder.h \
	src/font.cpp \
	src/font.h \
	src/fps_overlay.cpp \
	src/fps_overlay.h \
	src/frame.cpp \
	src/frame.h \
	src/game_actor.cpp \
	src/game_actor.h \
	src/game_actors.cpp \
	src/game_actors.h \
	src/game_battle.cpp \
	src/game_battle.h \
	src/game_battlealgorithm.cpp \
	src/game_battlealgorithm.h \
	src/game_battler.cpp \
	src/game_battler.h \
	src/game_character.cpp \
	src/game_character.h \
	src/game_commonevent.cpp \
	src/game_commonevent.h \
	src/game_enemy.cpp \
	src/game_enemy.h \
	src/game_enemyparty.cpp \
	src/game_enemyparty.h \
	src/game_event.cpp \
	src/game_event.h \
	src/game_interpreter_battle.cpp \
	src/game_interpreter_battle.h \
	src/game_interpreter.cpp \
	src/game_interpreter.h \
	src/game_interpreter_map.cpp \
	src/game_interpreter_map.h \
	src/game_map.cpp \
	src/game_map.h \
	src/game_message.cpp \
	src/game_message.h \
	src/game_party.cpp \
	src/game_party.h \
	src/game_party_base.cpp \
	src/game_party_base.h \
	src/game_picture.cpp \
	src/game_picture.h \
	src/game_player.cpp \
	src/game_player.h \
	src/game_screen.cpp \
	src/game_screen.h \
	src/game_switches.cpp \
	src/game_switches.h \
	src/game_system.cpp \
	src/game_system.h \
	src/game_targets.cpp \
	src/game_targets.h \
	src/game_temp.cpp \
	src/game_temp.h \
	src/game_variables.cpp \
	src/game_variables.h \
	src/game_vehicle.cpp \
	src/game_vehicle.h \
	src/graphics.cpp \
	src/graphics.h \
	src/hslrgb.cpp \
	src/hslrgb.h \
	src/icon.h \
	src/image_bmp.cpp \
	src/image_bmp.h \
	src/image_png.cpp \
	src/image_png.h \
	src/image_xyz.cpp \
	src/image_xyz.h \
	src/input_buttons_desktop.cpp \
	src/input_buttons_gekko.cpp \
	src/input_buttons.h \
	src/input_buttons_opendingux.cpp \
	src/input_buttons_psp.cpp \
	src/input_source.cpp \
	src/input_source.h \
	src/input.cpp \
	src/input.h \
	src/keys.h \
	src/logo.h \
	src/main_data.cpp \
	src/main_data.h \
	src/map_data.h \
	src/memory_management.h \
	src/message_overlay.cpp \
	src/message_overlay.h \
	src/options.h \
	src/output.cpp \
	src/output.h \
	src/picojson.h \
	src/pixel_format.h \
	src/plane.cpp \
	src/plane.h \
	src/player.cpp \
	src/player.h \
	src/rect.cpp \
	src/rect.h \
	src/registry.cpp \
	src/registry_wine.cpp \
	src/registry.h \
	src/rtp_table.cpp \
	src/rtp_table.h \
	src/scene_actortarget.cpp \
	src/scene_actortarget.h \
	src/scene_battle.cpp \
	src/scene_battle.h \
	src/scene_battle_rpg2k.cpp \
	src/scene_battle_rpg2k.h \
	src/scene_battle_rpg2k3.cpp \
	src/scene_battle_rpg2k3.h \
	src/scene.cpp \
	src/scene_debug.cpp \
	src/scene_debug.h \
	src/scene_end.cpp \
	src/scene_end.h \
	src/scene_equip.cpp \
	src/scene_equip.h \
	src/scene_file.cpp \
	src/scene_file.h \
	src/scene_gamebrowser.cpp \
	src/scene_gamebrowser.h \
	src/scene_gameover.cpp \
	src/scene_gameover.h \
	src/scene.h \
	src/scene_item.cpp \
	src/scene_item.h \
	src/scene_load.cpp \
	src/scene_load.h \
	src/scene_logo.cpp \
	src/scene_logo.h \
	src/scene_map.cpp \
	src/scene_map.h \
	src/scene_menu.cpp \
	src/scene_menu.h \
	src/scene_name.cpp \
	src/scene_name.h \
	src/scene_order.cpp \
	src/scene_order.h \
	src/scene_save.cpp \
	src/scene_save.h \
	src/scene_shop.cpp \
	src/scene_shop.h \
	src/scene_skill.cpp \
	src/scene_skill.h \
	src/scene_status.cpp \
	src/scene_status.h \
	src/scene_teleport.cpp \
	src/scene_teleport.h \
	src/scene_title.cpp \
	src/scene_title.h \
	src/screen.cpp \
	src/screen.h \
	src/sdl_ui.cpp \
	src/sdl_ui.h \
	src/bitmapfont.h \
	src/shinonome_gothic.cpp \
	src/shinonome_mincho.cpp \
	src/bitmapfont_wqy.cpp \
	src/bitmapfont_ttyp0.cpp \
	src/bitmapfont_rmg2000.cpp \
	src/sprite_airshipshadow.h \
	src/sprite_airshipshadow.cpp \
	src/sprite_battler.cpp \
	src/sprite_battler.h \
	src/sprite_character.cpp \
	src/sprite_character.h \
	src/sprite_timer.cpp \
	src/sprite_timer.h \
	src/sprite.cpp \
	src/sprite.h \
	src/spriteset_battle.cpp \
	src/spriteset_battle.h \
	src/spriteset_map.cpp \
	src/spriteset_map.h \
	src/system.h \
	src/text.cpp \
	src/text.h \
	src/tilemap.cpp \
	src/tilemap.h \
	src/tilemap_layer.cpp \
	src/tilemap_layer.h \
	src/tone.cpp \
	src/tone.h \
	src/transform.h \
	src/util_macro.h \
	src/utils.cpp \
	src/utils.h \
	src/util_win.cpp \
	src/util_win.h \
	src/version.h \
	src/weather.cpp \
	src/weather.h \
	src/window_about.cpp \
	src/window_about.h \
	src/window_actorinfo.cpp \
	src/window_actorinfo.h \
	src/window_actorstatus.cpp \
	src/window_actorstatus.h \
	src/window_actortarget.cpp \
	src/window_actortarget.h \
	src/window_base.cpp \
	src/window_base.h \
	src/window_battlecommand.cpp \
	src/window_battlecommand.h \
	src/window_battlemessage.cpp \
	src/window_battlemessage.h \
	src/window_battleoption.cpp \
	src/window_battleoption.h \
	src/window_battlestatus.cpp \
	src/window_battlestatus.h \
	src/window_command.cpp \
	src/window_command.h \
	src/window.cpp \
	src/window_equip.cpp \
	src/window_equip.h \
	src/window_equipitem.cpp \
	src/window_equipitem.h \
	src/window_equipstatus.cpp \
	src/window_equipstatus.h \
	src/window_face.cpp \
	src/window_face.h \
	src/window_gamelist.cpp \
	src/window_gamelist.h \
	src/window_gold.cpp \
	src/window_gold.h \
	src/window.h \
	src/window_help.cpp \
	src/window_help.h \
	src/window_item.cpp \
	src/window_item.h \
	src/window_keyboard.cpp \
	src/window_keyboard.h \
	src/window_menustatus.cpp \
	src/window_menustatus.h \
	src/window_message.cpp \
	src/window_message.h \
	src/window_name.cpp \
	src/window_name.h \
	src/window_numberinput.cpp \
	src/window_numberinput.h \
	src/window_savefile.cpp \
	src/window_savefile.h \
	src/window_selectable.cpp \
	src/window_selectable.h \
	src/window_shopbuy.cpp \
	src/window_shopbuy.h \
	src/window_shop.cpp \
	src/window_shop.h \
	src/window_shopnumber.cpp \
	src/window_shopnumber.h \
	src/window_shopparty.cpp \
	src/window_shopparty.h \
	src/window_shopsell.cpp \
	src/window_shopsell.h \
	src/window_shopstatus.cpp \
	src/window_shopstatus.h \
	src/window_skill.cpp \
	src/window_skill.h \
	src/window_skillstatus.cpp \
	src/window_skillstatus.h \
	src/window_targetstatus.cpp \
	src/window_targetstatus.h \
	src/window_teleport.cpp \
	src/window_teleport.h \
	src/window_varlist.cpp \
	src/window_varlist.h

if WANT_FMMIDI
libeasyrpg_player_la_SOURCES += \
	src/midiprogram.h \
	src/midisequencer.cpp \
	src/midisequencer.h \
	src/midisynth.cpp \
	src/midisynth.h
endif

libeasyrpg_player_la_CXXFLAGS = \
	-I$(srcdir)/src \
	-std=c++11 \
	$(LCF_CFLAGS) \
	$(PIXMAN_CFLAGS) \
	$(FREETYPE_CFLAGS) \
	$(HARFBUZZ_CFLAGS) \
	$(SDL_CFLAGS) \
	$(SDLMIXER_CFLAGS) \
	$(PNG_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(MPG123_CFLAGS) \
	$(WILDMIDI_CFLAGS) \
	$(OGGVORBIS_CFLAGS) \
	$(OPUS_CFLAGS) \
	$(SNDFILE_CFLAGS) \
	$(XMP_CFLAGS) \
	$(SPEEXDSP_CFLAGS)
libeasyrpg_player_la_LIBADD = \
	$(LCF_LIBS) \
	$(PIXMAN_LIBS) \
	$(FREETYPE_LIBS) \
	$(HARFBUZZ_LIBS) \
	$(SDL_LIBS) \
	$(SDLMIXER_LIBS) \
	$(PNG_LIBS) \
	$(ZLIB_LIBS) \
	$(MPG123_LIBS) \
	$(WILDMIDI_LIBS) \
	$(OGGVORBIS_LIBS) \
	$(OPUS_LIBS) \
	$(SNDFILE_LIBS) \
	$(XMP_LIBS) \
	$(SPEEXDSP_LIBS)

easyrpg_player_SOURCES = src/main.cpp
easyrpg_player_CXXFLAGS = $(libeasyrpg_player_la_CXXFLAGS)
easyrpg_player_LDADD = libeasyrpg-player.la

# manual page
if HAVE_A2X
resources/easyrpg-player.6: resources/easyrpg-player.6.adoc
	$(AM_V_GEN) $(A2X) -a player_version=$(PACKAGE_VERSION) -f manpage -D $(builddir)/resources $<

dist_man6_MANS = resources/easyrpg-player.6
MOSTLYCLEANFILES += resources/easyrpg-player.6
else
if HAVE_MANUAL
dist_man6_MANS = resources/easyrpg-player.6
endif
endif

# bash completion
if HAVE_BASHCOMPLETION
bashcompletiondir = $(BASHCOMPLETION_DIR)
dist_bashcompletion_DATA = resources/unix/bash-completion/easyrpg-player
endif

# FIXME make filefinder work without external scripting
check_PROGRAMS = output utils directorytree
TESTS = output utils directorytree
#filefinder_SOURCES = tests/filefinder.cpp
#filefinder_CXXFLAGS = $(libeasyrpg_player_la_CXXFLAGS)
#filefinder_LDADD = $(easyrpg_player_LDADD)
output_SOURCES = tests/output.cpp
output_CXXFLAGS = $(libeasyrpg_player_la_CXXFLAGS)
output_LDADD = $(easyrpg_player_LDADD)
utils_SOURCES = tests/utils.cpp
utils_CXXFLAGS = $(libeasyrpg_player_la_CXXFLAGS)
utils_LDADD = $(easyrpg_player_LDADD)
directorytree_SOURCES = tests/directorytree.cpp
directorytree_CXXFLAGS = $(libeasyrpg_player_la_CXXFLAGS)
directorytree_LDADD = $(easyrpg_player_LDADD)

# Some tests will create this file
# make distcheck will fail if it is not cleaned after runing these tests
CLEANFILES = easyrpg_log.txt
