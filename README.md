# EasyRPG Player

EasyRPG Player is a game interpreter to play RPG Maker 2000, 2003 and EasyRPG
games. It uses the LCF parser library (liblcf) to read RPG Maker game data.

EasyRPG Player is part of the EasyRPG Project. More information is
available at the project website: https://easyrpg.org/


## Documentation

Documentation is available at the documentation wiki: https://wiki.easyrpg.org


## Requirements

### minimal / required

- [liblcf] for RPG Maker data reading.
- SDL3 or SDL2 >= 2.0.14 for screen backend support.
- Pixman for low level pixel manipulation.
- libpng for PNG image support.
- zlib for XYZ image and ZIP archive support.
- fmtlib >= 6 for text formatting/coloring and internal logging.

### extended / recommended

- FreeType2 for external font support (+ HarfBuzz for Unicode text shaping).
- mpg123 for MP3 audio support.
- WildMIDI for MIDI audio support using GUS patches.
- FluidSynth for MIDI audio support using soundfonts.
- Libvorbis / Tremor for Ogg Vorbis audio support.
- opusfile for Opus audio support.
- libsndfile for better WAVE audio support.
- libxmp for tracker music support.
- SpeexDSP or libsamplerate for proper audio resampling.
- lhasa for LHA (.lzh) archive support.
- nlohmann_json for processing JSON files (required when targeting Emscripten)

The older SDL version 1.2 is still supported, but deprecated.
Please do not add new platform code for this library.


## Daily builds

Up to date binaries for assorted platforms are available at our continuous
integration service:

https://ci.easyrpg.org/view/Player/


## Source code

EasyRPG Player development is hosted by GitHub, project files are available
in this git repository:

https://github.com/EasyRPG/Player

Released versions are also available at our Download Archive:

https://easyrpg.org/downloads/player/


## Building

See [BUILDING document].


## Running EasyRPG Player

Run the `easyrpg-player` executable from a RPG Maker 2000 or 2003 game
project folder (same place as `RPG_RT.exe`).


## Bug reporting

Available options:

* File an issue at https://github.com/EasyRPG/Player/issues
* Open a thread at https://community.easyrpg.org/
* Chat with us via IRC: [#easyrpg at irc.libera.chat]


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
[baekmuk]: https://kldp.net/baekmuk
[Shinonome]: http://openlab.ring.gr.jp/efont/shinonome
[ttyp0]: https://people.mpi-inf.mpg.de/~uwe/misc/uw-ttyp0
[WenQuanYi]: http://wenq.org
[Teenyicons]: https://github.com/teenyicons/teenyicons
