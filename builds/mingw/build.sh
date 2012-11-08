#!/bin/bash -e

START_DIR=$(pwd)

MINGW_TARGET="i686-w64-mingw32"

export CC="ccache $MINGW_TARGET-gcc"
export CXX="ccache $MINGW_TARGET-g++"
export RANLIB="$MINGW_TARGET-ranlib"
export STRIP="$MINGW_TARGET-strip"
export NM="$MINGW_TARGET-nm"

cd $(dirname $0)
SCRIPT_DIR=$(pwd)
BUILD_DIR=$SCRIPT_DIR/build

export HOME_MINGW_PATH="$SCRIPT_DIR/mingw"

export CFLAGS=-I$HOME_MINGW_PATH/include
export CXXFLAGS=-I$HOME_MINGW_PATH/include
export LDFLAGS=-L$HOME_MINGW_PATH/lib
export LIBS=-lm

CONFIGURE_FLAGS="--host=$MINGW_TARGET
--prefix=$HOME_MINGW_PATH
--enable-shared=no
--enable-static=yes"

rm -Rf $BUILD_DIR

if ! [ -d $HOME_MINGW_PATH ] ; then
    mkdir $HOME_MINGW_PATH
    mkdir $HOME_MINGW_PATH/include
fi

if ! [ -d $SCRIPT_DIR/archive ] ; then
    mkdir $SCRIPT_DIR/archive
fi

JOBS=2

mkdir $BUILD_DIR
cd $BUILD_DIR

# x264
if ! [ -f $HOME_MINGW_PATH/include/x264.h ] ; then
    git clone git://git.videolan.org/x264.git
    cd x264
    ./configure $CONFIGURE_FLAGS
    make
    make install install-lib-dev install-lib-static
    cd $BUILD_DIR
fi

# winpthreads
if ! [ -f $HOME_MINGW_PATH/include/pthread.h ] ; then
    svn checkout "https://mingw-w64.svn.sourceforge.net/svnroot/mingw-w64/experimental/winpthreads"
    cd winpthreads
    ./configure $CONFIGURE_FLAGS
    make
    make install
fi

# zlib
if ! [ -f $HOME_MINGW_PATH/include/zlib.h ] ; then
    ZLIB_VERSION=1.2.7
    wget --continue \
        "http://sourceforge.net/projects/libpng/files/zlib/$ZLIB_VERSION/zlib-$ZLIB_VERSION.tar.gz/download" \
        -O $SCRIPT_DIR/archive/zlib-$ZLIB_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/zlib-$ZLIB_VERSION.tar.gz
    cd zlib-$ZLIB_VERSION
    ./configure --prefix=$HOME_MINGW_PATH --static

    sed -i -e "s/\/usr\/bin\/libtool/$MINGW_TARGET-ar rvs/" Makefile

    make libz.a
	  make install
    cd $BUILD_DIR
fi

# png
if ! [ -f $HOME_MINGW_PATH/include/png.h ] ; then
    PNG_VERSION=1.5.13
    wget --continue \
        "http://sourceforge.net/projects/libpng/files/libpng15/$PNG_VERSION/libpng-$PNG_VERSION.tar.gz/download" \
        -O $SCRIPT_DIR/archive/libpng-$PNG_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/libpng-$PNG_VERSION.tar.gz
    cd libpng-$PNG_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# expat
if ! [ -f $HOME_MINGW_PATH/include/expat.h ] ; then
    EXPAT_VERSION=2.1.0
    wget --continue \
        "http://sourceforge.net/projects/expat/files/expat/$EXPAT_VERSION/expat-$EXPAT_VERSION.tar.gz/download" \
        -O $SCRIPT_DIR/archive/expat-$EXPAT_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/expat-$EXPAT_VERSION.tar.gz
    cd expat-$EXPAT_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# jpeg
if ! [ -f $HOME_MINGW_PATH/include/jpeglib.h ] ; then
    wget --continue \
        "http://www.ijg.org/files/jpegsrc.v8d.tar.gz" \
        -O $SCRIPT_DIR/archive/jpegsrc.v8d.tar.gz
    tar -xf $SCRIPT_DIR/archive/jpegsrc.v8d.tar.gz
    cd jpeg-8d
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# SDL
if ! [ -f $HOME_MINGW_PATH/include/SDL/SDL.h ] ; then
    SDL_VERSION=1.2.15
    wget --continue \
        "http://www.libsdl.org/release/SDL-$SDL_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/SDL-$SDL_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/SDL-$SDL_VERSION.tar.gz
    cd SDL-$SDL_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# libogg
if ! [ -f $HOME_MINGW_PATH/include/ogg/ogg.h ] ; then
    OGG_VERSION=1.3.0
    wget --continue \
        "http://downloads.xiph.org/releases/ogg/libogg-$OGG_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/libogg-$OGG_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/libogg-$OGG_VERSION.tar.gz
    cd libogg-$OGG_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# libmikmod
if ! [ -f $HOME_MINGW_PATH/include/mikmod.h ] ; then
    LIBMIKMOD_VERSION=3.2.0
    wget --continue \
        "http://mikmod.shlomifish.org/files/libmikmod-$LIBMIKMOD_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/libmikmod-$LIBMIKMOD_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/libmikmod-$LIBMIKMOD_VERSION.tar.gz
    cd libmikmod-$LIBMIKMOD_VERSION

    sed -i -e "s/-Dunix//" libmikmod/Makefile.in
    sed -i -e "s/`uname`/MinGW/g" configure
    sed -i -e "s/defined _DLL/defined _DLL_NO/" */mikmod.h.in
    CC="$CC -msse2" ./configure $CONFIGURE_FLAGS --disable-shared --disable-esd

    make -j $JOBS
	  make install

    cd $BUILD_DIR
fi

# FLAC
if ! [ -f $HOME_MINGW_PATH/include/FLAC/all.h ] ; then
    FLAC_VERSION=1.2.1
    wget --continue \
        "http://sourceforge.net/projects/flac/files/flac-src/flac-$FLAC_VERSION-src/flac-$FLAC_VERSION.tar.gz/download" \
        -O $SCRIPT_DIR/archive/flac-$FLAC_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/flac-$FLAC_VERSION.tar.gz
    cd flac-$FLAC_VERSION

    sed -i -e "s/SUBDIRS = doc include m4 man src examples test build obj/SUBDIRS = doc include m4 man src build obj/" Makefile.*

    ./configure $CONFIGURE_FLAGS

    make -j $JOBS all-am
	  make install
    cd $BUILD_DIR
fi

# libvorbis
if ! [ -f $HOME_MINGW_PATH/include/vorbis/vorbisfile.h ] ; then
    VORBIS_VERSION=1.3.3
    wget --continue \
        "http://downloads.xiph.org/releases/vorbis/libvorbis-$VORBIS_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/libvorbis-$VORBIS_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/libvorbis-$VORBIS_VERSION.tar.gz
    cd libvorbis-$VORBIS_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

<<SDL_MIXER
# smpeg
if ! [ -f $HOME_MINGW_PATH/include/smpeg/smpeg.h ] ; then
    svn checkout "svn://svn.icculus.org/smpeg/trunk" smpeg
    cd smpeg
    ./autogen.sh
    ./configure $CONFIGURE_FLAGS --disable-gtktest --disable-gtk-player --disable-shared
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# sdl_mixer
if ! [ -f $HOME_MINGW_PATH/include/SDL/SDL_mixer.h ] \
    || ! [ -f $HOME_MINGW_PATH/include/SDL/SDL.h] \
    || ! [ -f $HOME_MINGW_PATH/include/ogg/ogg.h] \
    || ! [ -f $HOME_MINGW_PATH/include/mikmod.h] \
    || ! [ -f $HOME_MINGW_PATH/include/FLAC/all.h] \
    || ! [ -f $HOME_MINGW_PATH/include/vorbis/vorbisfile.h] \
    ; then
    SDL_MIXER_VERSION=1.2.12
    wget --continue \
        "http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-$SDL_MIXER_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/SDL_mixer-$SDL_MIXER_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/SDL_mixer-$SDL_MIXER_VERSION.tar.gz
    cd SDL_mixer-$SDL_MIXER_VERSION
    ./configure $CONFIGURE_FLAGS --disable-music-mp3 --target=i686-w64-mingw32
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi
SDL_MIXER

# Freetype
if ! [ -f $HOME_MINGW_PATH/include/ft2build.h ] ; then
    FREETYPE_VERSION=2.4.10
    wget --continue \
        "http://download.savannah.gnu.org/releases/freetype/freetype-$FREETYPE_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/freetype-$FREETYPE_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/freetype-$FREETYPE_VERSION.tar.gz
    cd freetype-$FREETYPE_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# pixman
if ! [ -f $HOME_MINGW_PATH/include/pixman-1/pixman.h ] ; then
    PIXMAN_VERSION=0.28.0
    wget --continue \
        "http://cairographics.org/releases/pixman-$PIXMAN_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/pixman-$PIXMAN_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/pixman-$PIXMAN_VERSION.tar.gz
    cd pixman-$PIXMAN_VERSION
    sed -i -e "s/SUBDIRS = pixman demos test/SUBDIRS = pixman/" Makefile.in

    unset CC
    unset CXX
    unset RANLIB

    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# Lua
if ! [ -f $HOME_MINGW_PATH/include/lua.h ] ; then
    LUA_VERSION=5.2.1
    wget --continue \
        "http://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/lua-$LUA_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/lua-$LUA_VERSION.tar.gz
    cd lua-$LUA_VERSION

    sed -i -e "s/CC= gcc/CC=ccache $MINGW_TARGET-gcc/" src/Makefile
    export ESCAPED_HOME_MINGW_PATH=$(echo $HOME_MINGW_PATH | sed -e "s/\//\\\\\//g")
    sed -i -e "s/INSTALL_TOP= \/usr\/local/INSTALL_TOP=$ESCAPED_HOME_MINGW_PATH/" Makefile
    sed -i -e "s/AR= ar/AR=$MINGW_TARGET-ar/" src/Makefile
    sed -i -e "s/RANLIB= ranlib/RANLIB=$MINGW_TARGET-ranlib/" src/Makefile
    sed -i -e "s/RANLIB=strip/RANLIB=$MINGW_TARGET-strip/" src/Makefile
    make -j $JOBS mingw

    sed -i -e "s/lua luac/lua.exe luac.exe/" Makefile
	  make install

    cd $BUILD_DIR
fi

<<OPENAL_SOFT
# OpenAL Soft
if ! [ -f $HOME_MINGW_PATH/include/AL/AL.h ] ; then
    OPENAL_VERSION=1.14
    wget --continue \
        "http://kcat.strangesoft.net/openal-releases/openal-soft-$OPENAL_VERSION.tar.bz2" \
        -O $SCRIPT_DIR/archive/openal-soft-$OPENAL_VERSION.tar.bz2
    tar -xf $SCRIPT_DIR/archive/openal-soft-$OPENAL_VERSION.tar.bz2
    cd openal-soft-$OPENAL_VERSION
fi

OPENAL_SOFT

# boost
if ! [ -f $HOME_MINGW_PATH/include/boost/shared_ptr.hpp ] ; then
    rm -rf $HOME_MINGW_PATH/include/boost
    if [ -d /usr/include/boost ] ; then
        cp -a /usr/include/boost $HOME_MINGW_PATH/include/boost
    elif [ -d /opt/local/include/boost ] ; then
        cp -a /opt/local/include/boost $HOME_MINGW_PATH/include/boost
    fi
    cd $BUILD_DIR
fi

make -C $SCRIPT_DIR

cd $START_DIR
