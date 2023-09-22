# EasyRPG Multiplayer Native

[简体中文](docs/README-ZH.md)

English is not my native language. Please correct me.

Implementation of EasyRPG multiplayer in C++, through which you can quickly obtain the client and server.

This project incorporated multiplayer client code from another project in the form of patches. So, this is a project based on EasyRPG/Player, and the multiplayer client code has been imported from ynoproject/ynoclient. For details, see the first commit of this project 44347b6 (Initial commit. Add multiplayer changes).

You might ask what is the server based on. It reuses the client code, the server is like a mirror of the client (in terms of code). The C2S and S2C namespaces are removed, the two types of the packet are merged into one, making them bidirectional. As a result, the readability of messages.h has improved a lot. In the future, we might try to use binary data structures to refactor the packet classes again, to resolve the issue of not being able to include specific delimiters in the data.

If you are interested in this project, you can clone the repository to your local to browse the code. If you don't know how to use Git, please refer to: [Git Manual](https://git-scm.com/book/en/v2)

Remember to switch to the master\_tmp branch, as many updates will be submitted there first.


## How to use

### Before the start

If you need to use the program but cannot compile it, you can go to the [Releases](https://github.com/monokotech/EasyRPG-Multiplayer-Native/releases) to download precompiled binaries. You need to choose the architecture that is suitable for the system you are currently using.

Open the terminal, which is cmd.exe for windows and Terminal.app for macOS.

And then use the file manager to drag the game folder to the terminal ([What is a game folder? See the FAQ below](#what-is-the-game-folder)).

Add `cd ` before the folder path. In cmd.exe, use `cd /d ` (/d means switching drive) to enter the game folder.

At this time you need to find the location of Player,
 then run the `<absolute path>/Player.exe` or the `<absolute path>/EasyRPG Player.app/Contents/MacOS/EasyRPG\ Player` for macOS.

The first run will display in full screen (this is the default behavior of EasyRPG Player).

If you don't like it, you can press F1 and select Video -> Fullscreen as OFF.

Finally go back and press `<Save Settings>` once, and return to the game menu.

Now, you can press F10 to bring up the Chat Ui, and F9 to toggle notifications (this is a layer within Player).

### Client Commands

`!help`

Show usage.

`!server [on, off]` (alias !srv)

You can run the server with this command.

As long as Player.exe can be run, the server can be started.

Servers can be started within a LAN (e.g. same WiFi or VPN), whether on mobile or computer.

`!connect [address]` (alias !c)

You can connect to the server with this command.

Enter the LAN IP, public IP, or domain name to connect to the server.

The status remains as "Connected", then it can be considered available.

`!disconnect` (alias !d)

Disconnect.

`!name [text]`

If you set !name \<unknown\> it will revert to the empty name.

`!chat [LOCAL, GLOBAL, CRYPT]` [CRYPT Password]

Switch chat visibility.

Enter !chat CRYPT [password] and a key will be generated.

After that, only clients with the same password can see the chat messages.

If the current visibility is CRYPT, it can be changed to other visibility, such as GLOBAL.

Enter !chat CRYPT again, no password is required and you can switch back to CRYPT.

What is CRYPT (encrypted chat)? [See the FAQ below](#what-is-chat-crypt).

`!log [LOCAL, GLOBAL, CRYPT]`

Enter a visibility to !log to hide or show messages in that visibility.

`!immersive` (alias !imm)

Toggle the immersive mode.

You can save the settings for the above commands via F1 -> \<Save Settings\>

The settings of the !connect, !name, !chat, and !immersive commands will be saved.

### Use Podman or Docker to run the server

Although this project uses podman, you can still use docker to do the same thing.

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

### Use the command line to run the server

Player.exe --server --bind-address 0.0.0.0[:port] --config-path /path/to/folder

Player.exe needs to be renamed with the corresponding executable filename based on your system.

Dedicated server example:

easyrpg-player-server --bind-address 0.0.0.0[:port] --config-path /path/to/file.ini

### Compile on linux

Arch Linux
```
# Install compilation tools
pacman -S gcc-libs cmake ccache

# Install dependencies of liblcf
pacman -S expat icu

# Install Player dependencies
# The first line is required, the second line is optional
pacman -S sdl2 pixman libpng zlib fmt
pacman -S harfbuzz mpg123 wildmidi libvorbis opusfile libsndfile libxmp speexdsp fluidsynth

# Compile
# You can decide whether to compile a dedicated server by adjusting `-DBUILD_CLIENT=on -DBUILD_SERVER=off`
cmake -B build -DBUILD_CLIENT=on -DBUILD_SERVER=off -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build -j${$(getconf _NPROCESSORS_ONLN):-2}
```

### Compile on macOS

```
# Install Homebrew
See: https://brew.sh

# Install compilation tools
brew install cmake ccache

# Install dependencies of liblcf
brew install expat icu4c

# Install Player dependencies
# The first line is required, the second line is optional
brew install sdl2 pixman libpng zlib fmt
brew install freetype mpg123 wildmidi libvorbis opusfile libsndfile speexdsp

# Compile
ICU_ROOT=$(brew --prefix)/opt/icu4c cmake -B build -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPLAYER_WITH_OPUS=off -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build -j${$(getconf _NPROCESSORS_ONLN):-2}
```

### Compile on Windows

Please refer to: [Guide: How To Build EasyRPG Player on windows](https://community.easyrpg.org/t/guide-how-to-build-easyrpg-player-on-windows/1174)

### More compilation examples

You can view the .github/workflows/releases.yml file.


## FAQ

### What is the game folder?

The game folder should include a lot of .lmu files, and may include the RPG\_RT.exe

### What is !chat CRYPT

The CRYPT using end-to-end encryption prevents chat messages from being captured by middlemen, such as servers, VPN servers or ISPs.

Encryption uses AES-256-GCM Authenticated Encryption.

The password will first be iterated 600000 times through PBKDF2 to obtain a 256bit key.

Then use the key to make an integer through CRC32 and passed to the server to match clients with the same password.

This key is saved to the configuration file as Base64.


The implementation is here: [chatui.cpp](https://github.com/monokotech/EasyRPG-Multiplayer-Native/blob/9b63310aa53409b5ac1c549c1a4601e5468c05f2/src/multiplayer/chatui.cpp#L881)

### How to make translation work?

Download the master.zip of ynotranslations and extract it:

https://github.com/ynoproject/ynotranslations/archive/refs/heads/master.zip

Find the folder as you need in the `ynotranslations`, then copy it to the game folder.
 Rename this copied folder as `Language` and restart the Player. After that, a new entry will appear in the game menu,
 enter and select language.

If you want to lock the language, you can pass --language \<name\> to the Player, the name
 you want is in the Language folder.

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
- [sock++] for multiplayer.
- [crypto++] for multiplayer chat.
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
- Char0x61 fork (https://github.com/CataractJustice/ynoclient)

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
[sock++]: https://github.com/fpagliughi/sockpp
[crypto++]: https://www.cryptopp.com/wiki/Main_Page
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
[chat_multiplayer.cpp]: https://github.com/CataractJustice/ynoclient/blob/master/src/chat_multiplayer.cpp
[strfnd.h]: https://github.com/minetest/minetest
[baekmuk]: https://kldp.net/baekmuk
[Shinonome]: http://openlab.ring.gr.jp/efont/shinonome
[ttyp0]: https://people.mpi-inf.mpg.de/~uwe/misc/uw-ttyp0
[WenQuanYi]: http://wenq.org
[Teenyicons]: https://github.com/teenyicons/teenyicons
