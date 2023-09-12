# EasyRPG Multiplayer Native

[简体中文](docs/README-ZH.md)

Add multiplayer functionality and the server in C++.


## Frequently Asked Questions

### How to create server with podman?

You can use docker, although podman is used in the project.

```
# Change directory
cd EasyRPG-Multiplayer-Native

# Build the image
podman build --build-arg TAG="$(git describe --tags)" -t epmp_img .

# Create the container
podman create --name epmp_container -p 6500:6500 epmp_img

# Start the container
podman restart epmp_container

# Remove the container
podman stop epmp_container && podman rm epmp_container
```

### How to build the Player on the macOS?

The project can be compiled and run on macOS, but the Opus cannot be enabled.

```
# Install dependencies of liblcf
brew install expat icu4c

# Install dependencies of Player
brew install libpng libvorbis sdl2 sdl2_mixer pixman freetype

# Build
ICU_ROOT=/opt/homebrew/opt/icu4c cmake -B build -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=Debug -DPLAYER_WITH_OPUS=off
cmake --build build

# If you have problems, you can revert back to previous state
brew unlink icu4c
```

### How to run on Windows?

You can get the precompiled binaries from here:

https://github.com/monokotech/EasyRPG-Multiplayer-Native/releases

The file `Windows-Build-*.zip` is what you need, only the Player.exe is
 needed inside the zip and copy it to the game folder.

### How to make translation work?

Download the master.zip of ynotranslations and extract it:

https://github.com/ynoproject/ynotranslations/archive/refs/heads/master.zip

Find the folder as you need in the ynotranslations and copy that to the game folder,
 and then rename it to `Language`. After that, a new entry will appear in the game menu,
 enter and select language.

If you want to lock the language, you can pass --language \<name\> to the Player, the name
 you want is in the Language folder.

### What is the game folder?

The game folder should include a lot of .lmu files, and may include the RPG\_RT.exe

### How to limit the frame rate?

To enable frame limiter requires to turn off V-Sync.

For older laptops, it is recommended to limit the frame rate below 20fps
 to minimize fan noise.

Press F1 -> Turn V-Sync Off -> Change Frame Limter to 20

### Why doesn't Save Settings work?

The Save Settings will not have any feedback when it is saved, but it outputs
 a log in the terminal with the path of config.ini. In fact, the settings are saved.

Also, the multiplayer settings are included in the config.ini, i.e. you can use
 client commands without the arguments.

### Is there a key to mute or unmute?

Yes, you can press the M key to toggle mute on and off.


## Requirements

### Required

- [liblcf] for RPG Maker data reading.
- SDL2 for screen backend support.
- Pixman for low level pixel manipulation.
- libpng for PNG image support.
- zlib for XYZ image support.
- fmtlib for interal logging.

### Optional

- FreeType2 for external font support (+ HarfBuzz for Unicode text shaping)
- mpg123 for better MP3 audio support
- WildMIDI for better MIDI audio support
- Libvorbis / Tremor for Ogg Vorbis audio support
- opusfile for Opus audio support
- libsndfile for better WAVE audio support
- libxmp for better tracker music support
- SpeexDSP for proper audio resampling

SDL 1.2 is still supported, but deprecated.


## Credits

### Projects

- EasyRPG developers - EasyRPG Player (https://github.com/EasyRPG/Player)
- YNOproject - Yume Nikki Online (https://github.com/ynoproject/ynoclient)

### Additional thanks

- [Jixun](https://github.com/jixunmoe) for helping in the C++ problems
- [Ratizux](https://github.com/Ratizux) for the podman suggestions
- [Proselyte093](https://github.com/Proselyte093) for giving the project a chance to compile on the macOS ARM
- ChatGPT for the C++ knowledge
- With help from various participants


## License

EasyRPG Player is free software available under the GPLv3 license. See the file
[COPYING] for license conditions. For Author information see [AUTHORS document].

EasyRPG [Logo] and [Logo2] are licensed under the CC-BY-SA 4.0 license.

### 3rd party software

EasyRPG Player makes use of the following 3rd party software:

* [FMMidi] YM2608 FM synthesizer emulator - Copyright (c) 2003-2006 yuno
  (Yoshio Uno), provided under the (3-clause) BSD license
* [dr_wav] WAV audio loader and writer - Copyright (c) David Reid, provided
  under public domain or MIT-0
* [PicoJSON] JSON parser/serializer - Copyright (c) 2009-2010 Cybozu Labs, Inc.
  Copyright (c) 2011-2015 Kazuho Oku, provided under the (2-clause) BSD license
* [rang] terminal color library - by Abhinav Gauniyal, provided under Unlicense
* [chat_multiplayer.cpp] In-game chat for SDL2 UI from CataractJustice/ynoclient,
  filename was renamed to chatui.cpp - by ledgamedev, provided under the GPLv3
* [sockpp] C++ socket library - Copyright (c) 2016-2017 Frank Pagliughi, provided
  under the (3-clause) BSD license
* [strfnd.h] Minetest string utils - Copyright (C) 2013 celeron55,
  Perttu Ahola \<celeron55@gmail.com\>, provided under the LGPLv2.1+

### 3rd party resources

* [Baekmuk] font family (Korean) - Copyright (c) 1986-2002 Kim Jeong-Hwan,
  provided under the Baekmuk License
* [Shinonome] font family (Japanese) - Copyright (c) 1999-2000 Yasuyuki
  Furukawa and contributors, provided under public domain. Glyphs were added
  and modified for use in EasyRPG Player, all changes under public domain.
* [ttyp0] font family - Copyright (c) 2012-2015 Uwe Waldmann, provided under
  ttyp0 license
* [WenQuanYi] font family (CJK) - Copyright (c) 2004-2010 WenQuanYi Project
  Contributors provided under the GPLv2 or later with Font Exception
* [Teenyicons] Tiny minimal 1px icons - Copyright (c) 2020 Anja van Staden,
  provided under the MIT license (only used by the Emscripten web shell)

[liblcf]: https://github.com/EasyRPG/liblcf
[BUILDING document]: docs/BUILDING.md
[#easyrpg at irc.libera.chat]: https://kiwiirc.com/nextclient/#ircs://irc.libera.chat/#easyrpg?nick=rpgguest??
[COPYING]: COPYING
[AUTHORS document]: docs/AUTHORS.md
[Logo]: resources/logo.png
[Logo2]: resources/logo2.png
[FMMidi]: http://unhaut.epizy.com/fmmidi
[dr_wav]: https://github.com/mackron/dr_libs
[PicoJSON]: https://github.com/kazuho/picojson
[rang]: https://github.com/agauniyal/rang
[chat_multiplayer.cpp]: https://github.com/CataractJustice/ynoclient
[sockpp]: https://github.com/fpagliughi/sockpp
[strfnd.h]: https://github.com/minetest/minetest
[baekmuk]: https://kldp.net/baekmuk
[Shinonome]: http://openlab.ring.gr.jp/efont/shinonome
[ttyp0]: https://people.mpi-inf.mpg.de/~uwe/misc/uw-ttyp0
[WenQuanYi]: http://wenq.org
[Teenyicons]: https://github.com/teenyicons/teenyicons
