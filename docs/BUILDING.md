# Building

## Dependencies

If your operating system has a package manager, we recommend installing the
dependencies with it.

In case you want to compile the dependencies yourself, you can find them,
except for [liblcf], in our [buildscripts] repository.


## Standard build with CMake

Building requirements:

- pkg-config (only on Linux)
- CMake 3.18 or newer

Step-by-step instructions:

    tar xf easyrpg-player-0.8.1.tar.xz    # unpack the tarball
    cd easyrpg-player-0.8.1               # enter in the package directory
    cmake . -DCMAKE_BUILD_TYPE=Release    # configure project
    cmake --build .                       # compile the executable
    sudo cmake --build . --target install # install system-wide

CMake is the only supported way to build Player. On Windows all dependencies
must be installed with [vcpkg].


## libretro core

Additional commands required before building:

    git submodule init   # Init submodules
    git submodule update # Clone libretro-common submodule

Invoke CMake with these additional parameters:

    -DPLAYER_TARGET_PLATFORM=libretro -DBUILD_SHARED_LIBS=ON|OFF

Set shared libs to ON or OFF depending on which type of libraries RetroArch
uses on the platform you are targeting.


## Android APK

Building requirements:

- Android SDK with NDK r21

Step-by-step instructions:

    tar xf easyrpg-player-0.8.1.tar.xz     # unpack the tarball
    cd easyrpg-player-0.8.1/builds/android # enter in the android directory
    ./gradlew -PtoolchainDirs="DIR1;DIR2" assembleRelease # create the APK

Replace ``DIR1`` etc. with the path to the player dependencies. You can use
the scripts in the ``android`` folder of our [buildscripts] to compile them.

To pass additional CMake arguments use ``-PcmakeOptions``:

    -PcmakeOptions="-DSOME_OPTION1=ON -DSOME_OPTION2=OFF"

The unsigned APK is stored in:

    app/build/outputs/apk/release/app-release-unsigned.apk


## Nintendo and Sony Homebrew ports (Wii (U), 3DS, Switch, PSVita/PSTV)

Building requirements:

- devkitPPC for Wii and Wii U
- devkitARM for 3DS
- devkitA64 for Switch
- vitasdk for PSVita/PSTV

You can get them at [devkitpro.org] and [vitasdk.org].

Invoke CMake with these additional parameters:

    -DCMAKE_TOOLCHAIN_FILE=<DEVKITPRO>/cmake/3DS|Switch|Wii|WiiU.cmake
                         (or <VITASDK>/share/vita.toolchain.cmake)

Switch, Wii U and 3DS support shipping games, use these parameters:

    -DPLAYER_BUNDLE=ON -DPLAYER_BUNDLE_PATH=path/to/myGame

## PlayStation Portable (PSP)

Requires the [pspdev] toolchain. Set `PSPDEV` to its install prefix, then:

    builds/psp/build-deps.sh       # zlib, libpng, pixman, fmt, inih, liblcf into $PSPDEV/psp
    cmake -S . -B build-psp -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=builds/psp/PspToolchain.cmake \
        -DBUILD_SHARED_LIBS=OFF -DPLAYER_BUILD_LIBLCF=OFF \
        -DPLAYER_WITH_FREETYPE=OFF -DPLAYER_WITH_HARFBUZZ=OFF -DPLAYER_WITH_LHASA=OFF \
        -DPLAYER_WITH_NLOHMANN_JSON=OFF -DPLAYER_WITH_BAEKMUK=OFF -DPLAYER_WITH_WQY=OFF
    cmake --build build-psp

The build produces `build-psp/psp/EBOOT.PBP`. Put it in a folder named
`EasyRPG` under `ms0:/PSP/GAME/` together with the game data. The
`builds/psp/build-release.sh` script runs all of this inside the pspdev
container and creates the release zip (see `.github/workflows/psp.yml`).

[pspdev]: https://github.com/pspdev/pspdev


[buildscripts]: https://github.com/EasyRPG/buildscripts
[liblcf]: https://github.com/EasyRPG/liblcf
[vcpkg]: https://github.com/Microsoft/vcpkg
[devkitpro.org]: https://devkitpro.org/wiki/Getting_Started
[vitasdk.org]: https://vitasdk.org
