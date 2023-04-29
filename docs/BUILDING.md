# Building

## Dependencies:

If your operating system has a package manager, we recommend installing the
dependencies with it.

In case you want to compile the dependencies yourself, you can find them,
except for [liblcf], in our [buildscripts] repository.


## Autotools Makefile method:

Building requirements:

- pkg-config
- GNU make

Step-by-step instructions:

    tar xf easyrpg-player-0.8.tar.xz # unpack the tarball
    cd easyrpg-player-0.8            # enter in the package directory
    ./configure                      # find libraries, set options
    make                             # compile the executable

Additional building requirements when using the source tree (git):

- autoconf >= 2.69
- automake >= 1.11.4
- libtool

To generate the "configure" script, run before following the above section:

    autoreconf -i


## CMake method:

Building requirements:

- pkg-config (only on Linux)
- CMake 3.10 or newer

Step-by-step instructions:

    tar xf easyrpg-player-0.8.tar.xz      # unpack the tarball
    cd easyrpg-player-0.8                 # enter in the package directory
    cmake . -DCMAKE_BUILD_TYPE=Release    # configure project
    cmake --build .                       # compile the executable
    sudo cmake --build . --target install # install system-wide

CMake is the only supported way to build Player for Windows. All dependencies
must be installed with [vcpkg].


## libretro core:

Building for libretro is based on the CMake method.

Additional commands required before building:

    git submodule init   # Init submodules
    git submodule update # Clone libretro-common submodule

Invoke CMake with these additional parameters:

    -DPLAYER_TARGET_PLATFORM=libretro -DBUILD_SHARED_LIBS=ON|OFF

Set shared libs to ON or OFF depending on which type of libraries RetroArch
uses on the platform you are targeting.


## Android APK:

Building requirements:

- Android SDK with NDK r21

Step-by-step instructions:

    tar xf easyrpg-player-0.8.tar.xz     # unpack the tarball
    cd easyrpg-player-0.8/builds/android # enter in the android directory
    ./gradlew -PtoolchainDirs="DIR1;DIR2" assembleRelease # create the APK

Replace ``DIR1`` etc. with the path to the player dependencies. You can use
the scripts in the ``android`` folder of our [buildscripts] to compile them.

To pass additional CMake arguments use ``-PcmakeOptions``:

    -PcmakeOptions="-DSOME_OPTION1=ON -DSOME_OPTION2=OFF"

The unsigned APK is stored in:

    app/build/outputs/apk/release/app-release-unsigned.apk


## Nintendo and Sony Homebrew ports (Wii, 3DS, Switch, PSVita/PSTV)

This is based on the CMake method.

Building requirements:

- devkitPPC for Wii
- devkitARM for 3DS
- devkitA64 for Switch
- vitasdk for PSVita/PSTV

You can get them at devkitPro: https://devkitpro.org/wiki/Getting_Started
and vitasdk: https://vitasdk.org

Invoke CMake with these additional parameters:

    -DCMAKE_TOOLCHAIN_FILE=<DEVKITPRO>/cmake/3DS|Switch|Wii.cmake
                         (or <VITASDK>/share/vita.toolchain.cmake)
    -DPLAYER_TARGET_PLATFORM=3ds|switch|wii|psvita

Switch and 3DS support romfs game loading, use these parameters:

    -DPLAYER_ROMFS=ON -DPLAYER_ROMFS_PATH=path/to/myGame


[buildscripts]: https://github.com/EasyRPG/buildscripts
[liblcf]: https://github.com/EasyRPG/liblcf
[vcpkg]: https://github.com/Microsoft/vcpkg
