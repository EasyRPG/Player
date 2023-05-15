ACLOCAL_AMFLAGS = --install -I builds/autoconf/m4

EXTRA_DIST = CMakeLists.txt README.md builds docs resources
MOSTLYCLEANFILES = $(DX_CLEANFILES)

bin_PROGRAMS = easyrpg-player

noinst_LIBRARIES = libeasyrpg-player.a libplayer-version.a
libeasyrpg_player_a_SOURCES = \
	src/lcf_data.cpp \
	src/lcf/data.h \
	src/async_handler.cpp \
	src/async_handler.h \
	src/async_op.h \
	src/algo.h \
	src/algo.cpp \
	src/attribute.h \
	src/attribute.cpp \
	src/audio.cpp \
	src/audio.h \
	src/audio_decoder.cpp \
	src/audio_decoder.h \
	src/audio_decoder_base.cpp \
	src/audio_decoder_base.h \
	src/audio_decoder_midi.cpp \
	src/audio_decoder_midi.h \
	src/audio_generic.cpp \
	src/audio_generic.h \
	src/audio_generic_midiout.cpp \
	src/audio_generic_midiout.h \
	src/audio_midi.cpp \
	src/audio_midi.h \
	src/audio_resampler.cpp \
	src/audio_resampler.h \
	src/audio_secache.cpp \
	src/audio_secache.h \
	src/autobattle.cpp \
	src/autobattle.h \
	src/background.cpp \
	src/background.h \
	src/baseui.cpp \
	src/baseui.h \
	src/battle_animation.cpp \
	src/battle_animation.h \
	src/battle_message.cpp \
	src/battle_message.h \
	src/bitmap.cpp \
	src/bitmap.h \
	src/bitmapfont.h \
	src/bitmapfont_glyph.h \
	src/bitmap_hslrgb.h \
	src/cache.cpp \
	src/cache.h \
	src/cmdline_parser.cpp \
	src/cmdline_parser.h \
	src/color.h \
	src/compiler.h \
	src/config_param.h \
	src/decoder_fluidsynth.cpp \
	src/decoder_fluidsynth.h \
	src/decoder_fmmidi.cpp \
	src/decoder_fmmidi.h \
	src/decoder_libsndfile.cpp \
	src/decoder_libsndfile.h \
	src/decoder_mpg123.cpp \
	src/decoder_mpg123.h \
	src/decoder_oggvorbis.cpp \
	src/decoder_oggvorbis.h \
	src/decoder_opus.cpp \
	src/decoder_opus.h \
	src/decoder_wildmidi.cpp \
	src/decoder_wildmidi.h \
	src/decoder_xmp.cpp \
	src/decoder_xmp.h \
	src/default_graphics.h \
	src/directory_tree.cpp \
	src/directory_tree.h \
	src/docmain.h \
	src/drawable.cpp \
	src/drawable.h \
	src/drawable_list.cpp \
	src/drawable_list.h \
	src/drawable_mgr.cpp \
	src/drawable_mgr.h \
	src/dynrpg.cpp \
	src/dynrpg.h \
	src/dynrpg_easyrpg.cpp \
	src/dynrpg_easyrpg.h \
	src/enemyai.cpp \
	src/enemyai.h \
	src/exe_reader.cpp \
	src/exe_reader.h \
	src/exfont.h \
	src/feature.cpp \
	src/feature.h \
	src/filefinder.cpp \
	src/filefinder.h \
	src/filefinder_rtp.cpp \
	src/filefinder_rtp.h \
	src/fileext_guesser.cpp \
	src/fileext_guesser.h \
	src/filesystem.cpp \
	src/filesystem.h \
	src/filesystem_native.cpp \
	src/filesystem_native.h \
	src/filesystem_root.cpp \
	src/filesystem_root.h \
	src/filesystem_stream.cpp \
	src/filesystem_stream.h \
	src/filesystem_zip.cpp \
	src/filesystem_zip.h \
	src/flash.h \
	src/flat_map.h \
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
	src/game_clock.cpp \
	src/game_clock.h \
	src/game_commonevent.cpp \
	src/game_commonevent.h \
	src/game_config.cpp \
	src/game_config.h \
	src/game_config_game.cpp \
	src/game_config_game.h \
	src/game_enemy.cpp \
	src/game_enemy.h \
	src/game_enemyparty.cpp \
	src/game_enemyparty.h \
	src/game_event.cpp \
	src/game_event.h \
	src/game_ineluki.cpp \
	src/game_ineluki.h \
	src/game_interpreter.cpp \
	src/game_interpreter.h \
	src/game_interpreter_battle.cpp \
	src/game_interpreter_battle.h \
	src/game_interpreter_control_variables.cpp \
	src/game_interpreter_control_variables.h \
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
	src/game_pictures.cpp \
	src/game_pictures.h \
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
	src/game_variables.cpp \
	src/game_variables.h \
	src/game_vehicle.cpp \
	src/game_vehicle.h \
	src/game_windows.cpp \
	src/game_windows.h \
	src/generated/bitmapfont_rmg2000.h \
	src/generated/bitmapfont_ttyp0.h \
	src/generated/bitmapfont_wqy.h \
	src/generated/logo.h \
	src/generated/logo2.h \
	src/generated/shinonome_gothic.h \
	src/generated/shinonome_mincho.h \
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
	src/input.cpp \
	src/input.h \
	src/input_buttons.h \
	src/input_buttons_desktop.cpp \
	src/input_source.cpp \
	src/input_source.h \
	src/instrumentation.cpp \
	src/instrumentation.h \
	src/keys.h \
	src/main_data.cpp \
	src/main_data.h \
	src/maniac_patch.cpp \
	src/maniac_patch.h \
	src/map_data.h \
	src/memory_management.h \
	src/message_overlay.cpp \
	src/message_overlay.h \
	src/meta.cpp \
	src/meta.h \
	src/midisequencer.cpp \
	src/midisequencer.h \
	src/opacity.h \
	src/options.h \
	src/output.cpp \
	src/output.h \
	src/pending_message.h \
	src/pending_message.cpp \
	src/pixel_format.h \
	src/pixman_image_ptr.h \
	src/plane.cpp \
	src/plane.h \
	src/platform.cpp \
	src/platform.h \
	src/platform/clock.h \
	src/player.cpp \
	src/player.h \
	src/point.h \
	src/game_quit.cpp \
	src/game_quit.h \
	src/rand.cpp \
	src/rand.h \
	src/rect.cpp \
	src/rect.h \
	src/registry.cpp \
	src/registry.h \
	src/registry_wine.cpp \
	src/rtp.cpp \
	src/rtp.h \
	src/rtp_table.cpp \
	src/scene.cpp \
	src/scene.h \
	src/scene_import.cpp \
	src/scene_import.h \
	src/scene_actortarget.cpp \
	src/scene_actortarget.h \
	src/scene_battle.cpp \
	src/scene_battle.h \
	src/scene_battle_rpg2k.cpp \
	src/scene_battle_rpg2k.h \
	src/scene_battle_rpg2k3.cpp \
	src/scene_battle_rpg2k3.h \
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
	src/scene_settings.cpp \
	src/scene_settings.h \
	src/scene_teleport.cpp \
	src/scene_teleport.h \
	src/scene_title.cpp \
	src/scene_title.h \
	src/screen.cpp \
	src/screen.h \
	src/shake.h \
	src/span.h \
	src/sprite.cpp \
	src/sprite.h \
	src/sprite_airshipshadow.h \
	src/sprite_airshipshadow.cpp \
	src/sprite_actor.cpp \
	src/sprite_actor.h \
	src/sprite_battler.cpp \
	src/sprite_battler.h \
	src/sprite_enemy.cpp \
	src/sprite_enemy.h \
	src/sprite_character.cpp \
	src/sprite_character.h \
	src/sprite_picture.cpp \
	src/sprite_picture.h \
	src/sprite_timer.cpp \
	src/sprite_timer.h \
	src/sprite_weapon.cpp \
	src/sprite_weapon.h \
	src/spriteset_battle.cpp \
	src/spriteset_battle.h \
	src/spriteset_map.cpp \
	src/spriteset_map.h \
	src/state.cpp \
	src/state.h \
	src/std_clock.h \
	src/string_view.cpp \
	src/string_view.h \
	src/system.h \
	src/teleport_target.h \
	src/text.cpp \
	src/text.h \
	src/tilemap.cpp \
	src/tilemap.h \
	src/tilemap_layer.cpp \
	src/tilemap_layer.h \
	src/tone.h \
	src/transform.h \
	src/transition.cpp \
	src/transition.h \
	src/translation.cpp \
	src/translation.h \
	src/util_macro.h \
	src/utils.cpp \
	src/utils.h \
	src/weather.cpp \
	src/weather.h \
	src/window.cpp \
	src/window.h \
	src/window_about.cpp \
	src/window_about.h \
	src/window_actorinfo.cpp \
	src/window_actorinfo.h \
	src/window_actorsp.cpp \
	src/window_actorsp.h \
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
	src/window_battlestatus.cpp \
	src/window_battlestatus.h \
	src/window_command.cpp \
	src/window_command.h \
	src/window_command_horizontal.cpp \
	src/window_command_horizontal.h \
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
	src/window_help.cpp \
	src/window_help.h \
	src/window_import_progress.cpp \
	src/window_import_progress.h \
	src/window_input_settings.cpp \
	src/window_input_settings.h \
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
	src/window_paramstatus.cpp \
	src/window_paramstatus.h \
	src/window_savefile.cpp \
	src/window_savefile.h \
	src/window_selectable.cpp \
	src/window_selectable.h \
	src/window_settings.cpp \
	src/window_settings.h \
	src/window_shop.cpp \
	src/window_shop.h \
	src/window_shopbuy.cpp \
	src/window_shopbuy.h \
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
	src/window_varlist.h \
	src/external/rang.hpp

SOURCEFILES_SDL2 = \
	src/platform/sdl/sdl2_ui.cpp \
	src/platform/sdl/sdl2_ui.h \
	src/platform/sdl/audio.cpp \
	src/platform/sdl/audio.h
if HAVE_SDL2
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_SDL2)
else
EXTRA_DIST += $(SOURCEFILES_SDL2)
endif

SOURCEFILES_SDL1 = \
	src/platform/sdl/sdl_ui.cpp \
	src/platform/sdl/sdl_ui.h \
	src/platform/sdl/audio.cpp \
	src/platform/sdl/audio.h \
	src/platform/sdl/axis.h
if HAVE_SDL1
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_SDL1)
else
EXTRA_DIST += $(SOURCEFILES_SDL1)
endif

SOURCEFILES_FMMIDI = \
	src/midiprogram.h \
	src/midisynth.cpp \
	src/midisynth.h
if WANT_FMMIDI
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_FMMIDI)
else
EXTRA_DIST += $(SOURCEFILES_FMMIDI)
endif

SOURCEFILES_DRWAV = \
	src/decoder_drwav.cpp \
	src/decoder_drwav.h \
	src/external/dr_wav.h
if WANT_DRWAV
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_DRWAV)
else
EXTRA_DIST += $(SOURCEFILES_DRWAV)
endif

SOURCEFILES_ALSA = \
	src/platform/linux/midiout_device_alsa.cpp \
	src/platform/linux/midiout_device_alsa.h
if HAVE_ALSA
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_ALSA)
else
EXTRA_DIST += $(SOURCEFILES_ALSA)
endif

SOURCEFILES_MACOS = \
	src/platform/macos/utils.h \
	src/platform/macos/utils.mm \
	src/platform/macos/midiout_device_coreaudio.cpp \
	src/platform/macos/midiout_device_coreaudio.h
if MACOS
libeasyrpg_player_a_SOURCES += $(SOURCEFILES_MACOS)
else
EXTRA_DIST += $(SOURCEFILES_MACOS)
endif

# These are used by CMake
EXTRA_DIST += \
	bench/bitmap.cpp \
	bench/draw.cpp \
	bench/font.cpp \
	bench/pixel_format.cpp \
	bench/rtp.cpp \
	bench/switches.cpp \
	bench/text.cpp \
	bench/utils.cpp \
	bench/variables.cpp \
	src/external/picojson.h \
	src/platform/3ds/audio.cpp \
	src/platform/3ds/audio.h \
	src/platform/3ds/clock.cpp \
	src/platform/3ds/clock.h \
	src/platform/3ds/input_buttons.cpp \
	src/platform/3ds/main.cpp \
	src/platform/3ds/ui.cpp \
	src/platform/3ds/ui.h \
	src/platform/amigaos4/cmath \
	src/platform/android/android.h \
	src/platform/android/filesystem_apk.cpp \
	src/platform/android/filesystem_apk.h \
	src/platform/android/filesystem_saf.cpp \
	src/platform/android/filesystem_saf.h \
	src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.cpp \
	src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.h \
	src/platform/emscripten/main.cpp \
	src/platform/libretro/audio.cpp \
	src/platform/libretro/audio.h \
	src/platform/libretro/clock.cpp \
	src/platform/libretro/clock.h \
	src/platform/libretro/input_buttons.cpp \
	src/platform/libretro/midiout_device.cpp \
	src/platform/libretro/midiout_device.h \
	src/platform/libretro/ui.cpp \
	src/platform/libretro/ui.h \
	src/platform/morphos/integration.cpp \
	src/platform/opendingux/opendingux_input_buttons.cpp \
	src/platform/psp/psp_input_buttons.cpp \
	src/platform/psvita/audio.cpp \
	src/platform/psvita/audio.h \
	src/platform/psvita/clock.cpp \
	src/platform/psvita/clock.h \
	src/platform/psvita/input_buttons.cpp \
	src/platform/psvita/main.cpp \
	src/platform/psvita/ui.cpp \
	src/platform/psvita/ui.h \
	src/platform/switch/audio.cpp \
	src/platform/switch/audio.h \
	src/platform/switch/clock.cpp \
	src/platform/switch/clock.h \
	src/platform/switch/input_buttons.cpp \
	src/platform/switch/main.cpp \
	src/platform/switch/ui.cpp \
	src/platform/switch/ui.h \
	src/platform/wii/clock.cpp \
	src/platform/wii/clock.h \
	src/platform/wii/input_buttons.cpp \
	src/platform/wii/main.cpp \
	src/platform/wii/main.h \
	src/platform/windows/midiout_device_win32.cpp \
	src/platform/windows/midiout_device_win32.h \
	src/platform/windows/utils.cpp \
	src/platform/windows/utils.h

libeasyrpg_player_a_CXXFLAGS = \
	-fno-math-errno \
	-I$(srcdir)/src \
	$(LCF_CFLAGS) \
	$(PIXMAN_CFLAGS) \
	$(FREETYPE_CFLAGS) \
	$(HARFBUZZ_CFLAGS) \
	$(SDL_CFLAGS) \
	$(PNG_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(LIBMPG123_CFLAGS) \
	$(LIBWILDMIDI_CFLAGS) \
	$(OGGVORBIS_CFLAGS) \
	$(OPUS_CFLAGS) \
	$(FMT_CFLAGS) \
	$(LIBSNDFILE_CFLAGS) \
	$(LIBXMP_CFLAGS) \
	$(LIBSPEEXDSP_CFLAGS) \
	$(FLUIDSYNTH_CFLAGS) \
	$(FLUIDLITE_CFLAGS) \
	$(ALSA_CFLAGS) \
	$(PTHREAD_CFLAGS)

libeasyrpg_player_a_OBJCXXFLAGS = $(libeasyrpg_player_a_CXXFLAGS)

# extra lib to only add version definitions here
libplayer_version_a_SOURCES = \
	src/version.cpp \
	src/version.h
libplayer_version_a_CXXFLAGS = \
	-DEP_VERSION_MAJOR=$(EP_VERSION_MAJOR) \
	-DEP_VERSION_MINOR=$(EP_VERSION_MINOR) \
	-DEP_VERSION_PATCH=$(EP_VERSION_PATCH) \
	-DEP_VERSION_TWEAK=$(EP_VERSION_TWEAK) \
	-DEP_VERSION='"$(EP_VERSION)"' \
	-DEP_VERSION_APPEND='"$(EP_VERSION_APPEND)"' \
	-DEP_VERSION_GIT=\"`cat $(builddir)/.version-git`\"

# regenerate lib when version has changed
$(libplayer_version_a_OBJECTS): $(builddir)/.version-append $(builddir)/.version-git

CLEANFILES = $(builddir)/.version-append $(builddir)/.version-git
.PHONY: update_version
$(builddir)/.version-append: update_version
	$(AM_V_at)[ -f $@ ] || touch $@
	$(AM_V_at)echo "$(EP_VERSION_APPEND)" | cmp -s $@ - || echo "$(EP_VERSION_APPEND)" > $@

$(builddir)/.version-git: update_version
	$(AM_V_at)[ -f $@ ] || touch $@
	$(AM_V_at)$(top_srcdir)/builds/autoconf/git-version.sh > $@-t
	$(AM_V_at)cmp -s $@ $@-t && rm $@-t || mv $@-t $@

easyrpg_player_SOURCES = src/platform/sdl/main.cpp
easyrpg_player_CXXFLAGS = $(libeasyrpg_player_a_CXXFLAGS)
easyrpg_player_LDADD = libeasyrpg-player.a libplayer-version.a \
	$(LCF_LIBS) \
	$(PIXMAN_LIBS) \
	$(FREETYPE_LIBS) \
	$(HARFBUZZ_LIBS) \
	$(SDL_LIBS) \
	$(PNG_LIBS) \
	$(ZLIB_LIBS) \
	$(LIBMPG123_LIBS) \
	$(LIBWILDMIDI_LIBS) \
	$(OGGVORBIS_LIBS) \
	$(OPUS_LIBS) \
	$(FMT_LIBS) \
	$(LIBSNDFILE_LIBS) \
	$(LIBXMP_LIBS) \
	$(LIBSPEEXDSP_LIBS) \
	$(FLUIDSYNTH_LIBS) \
	$(FLUIDLITE_LIBS) \
	$(ALSA_LIBS) \
	$(PTHREAD_LIBS)

if MACOS
easyrpg_player_LDFLAGS = -framework Foundation
endif

# manual page
if HAVE_ASCIIDOCTOR
resources/unix/easyrpg-player.6: resources/unix/easyrpg-player.6.adoc
	$(AM_V_GEN)$(ASCIIDOCTOR) -a player_version=$(PACKAGE_VERSION) -b manpage -D $(builddir)/resources/unix $<

dist_man6_MANS = resources/unix/easyrpg-player.6
MOSTLYCLEANFILES += resources/unix/easyrpg-player.6
else
if HAVE_MANUAL
dist_man6_MANS = resources/unix/easyrpg-player.6
endif
endif

# bash completion
if HAVE_BASHCOMPLETION
bashcompletiondir = $(BASHCOMPLETION_DIR)
dist_bashcompletion_DATA = resources/unix/bash-completion/easyrpg-player
endif

# desktop integration
if !MACOS
desktopdir = $(datadir)/applications
dist_desktop_DATA = resources/unix/easyrpg-player.desktop
metainfodir = $(datadir)/metainfo
dist_metainfo_DATA = resources/unix/easyrpg-player.metainfo.xml
icondir = $(datadir)/icons/hicolor
scalabledir = $(icondir)/scalable/apps
dist_scalable_DATA = resources/unix/easyrpg-player.svg
pixmapdir = $(datadir)/pixmaps

install-data-hook:
	@echo " $(MKDIR_P) '$(DESTDIR)$(pixmapdir)'"; \
	$(MKDIR_P) "$(DESTDIR)$(pixmapdir)" || exit 1; \
	echo " $(INSTALL_DATA) resources/unix/icon-48.png '$(DESTDIR)$(pixmapdir)/easyrpg-player.png'"; \
	$(INSTALL_DATA) $(srcdir)/resources/unix/icon-48.png "$(DESTDIR)$(pixmapdir)/easyrpg-player.png" || exit $$?; \
	for files in $(srcdir)/resources/unix/icon-*.png; do \
	  size=`basename $$files .png | tr -cd '[0-9]'`; \
	  echo " $(MKDIR_P) '$(DESTDIR)$(icondir)/$${size}x$${size}/apps'"; \
	  $(MKDIR_P) "$(DESTDIR)$(icondir)/$${size}x$${size}/apps" || exit 1; \
	  echo " $(INSTALL_DATA) resources/unix/icon-$${size}.png; '$(DESTDIR)$(icondir)/$${size}x$${size}/apps/easyrpg-player.png'"; \
	  $(INSTALL_DATA) $$files "$(DESTDIR)$(icondir)/$${size}x$${size}/apps/easyrpg-player.png" || exit $$?; \
	done
endif

# doxygen
@DX_RULES@

# tests
EXTRA_DIST += tests/assets

check_PROGRAMS = test_runner
test_runner_SOURCES = \
	tests/algo.cpp \
	tests/attribute.cpp \
	tests/autobattle.cpp \
	tests/bitmapfont.cpp \
	tests/cmdline_parser.cpp \
	tests/config_param.cpp \
	tests/doctest.h \
	tests/drawable_list.cpp \
	tests/drawable_mgr.cpp \
	tests/dynrpg.cpp \
	tests/enemyai.cpp \
	tests/filefinder.cpp \
	tests/filesystem.cpp \
	tests/filesystem_zip.cpp \
	tests/flat_map.cpp \
	tests/font.cpp \
	tests/game_actor.cpp \
	tests/game_battlealgorithm.cpp \
	tests/game_character.cpp \
	tests/game_character_anim.cpp \
	tests/game_character_flash.cpp \
	tests/game_character_move.cpp \
	tests/game_character_moveto.cpp \
	tests/game_enemy.cpp \
	tests/game_event.cpp \
	tests/game_player_input.cpp \
	tests/game_player_pan.cpp \
	tests/game_player_savecount.cpp \
	tests/mock_game.cpp \
	tests/mock_game.h \
	tests/move_route.cpp \
	tests/output.cpp \
	tests/parse.cpp \
	tests/platform.cpp \
	tests/rand.cpp \
	tests/rtp.cpp \
	tests/switches.cpp \
	tests/test_main.cpp \
	tests/test_mock_actor.h \
	tests/test_move_route.h \
	tests/text.cpp \
	tests/utf.cpp \
	tests/utils.cpp \
	tests/variables.cpp \
	tests/wordwrap.cpp

test_runner_CXXFLAGS = \
	$(libeasyrpg_player_a_CXXFLAGS) -DEP_TEST_PATH=\"$(canonical_srcdir)/tests/assets\"
test_runner_LDADD = \
	$(easyrpg_player_LDADD)

check-local:
	$(AM_V_at)./test_runner

# Some tests will create this file
# make distcheck will fail if it is not cleaned after running these tests
CLEANFILES += easyrpg_log.txt
