# EasyRPG Multiplayer Native

Add multiplayer functionality and the server in C++.


## Requirements

### minimal / required

- [liblcf] for RPG Maker data reading.
- sockpp for TCP socket.
- SDL2 for screen backend support.
- Pixman for low level pixel manipulation.
- libpng for PNG image support.
- zlib for XYZ image support.
- fmtlib for interal logging.

### extended / optional / recommended

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

- EasyRPG developers - EasyRPG Player (https://github.com/EasyRPG/Player)
- YNOproject - Yume Nikki Online (https://github.com/ynoproject/ynoclient)
- CataractJustice - EasyRPG Player online fork (https://github.com/CataractJustice/ynoclient)
- fpagliughi - Simple, modern, C++ socket library (https://github.com/fpagliughi/sockpp)


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
[baekmuk]: https://kldp.net/baekmuk
[Shinonome]: http://openlab.ring.gr.jp/efont/shinonome
[ttyp0]: https://people.mpi-inf.mpg.de/~uwe/misc/uw-ttyp0
[WenQuanYi]: http://wenq.org
[Teenyicons]: https://github.com/teenyicons/teenyicons
