#!/bin/sh

START_DIR=$(pwd)

export CC="ccache i686-w64-mingw32-gcc"
export CXX="ccache i686-w64-mingw32-g++"
export RANLIB=i686-w64-mingw32-ranlib

MINGW_TARGET="i686-w64-mingw32"

cd $(dirname $0)
SCRIPT_DIR=$(pwd)
BUILD_DIR=$SCRIPT_DIR/build

export HOME_MINGW_PATH="$SCRIPT_DIR/mingw"

CONFIGURE_FLAGS="--host=$MINGW_TARGET
--prefix=$HOME_MINGW_PATH
--enable-shared=no
--enable-static=yes
CFLAGS=-I$HOME_MINGW_PATH/include
CXXFLAGS=-I$HOME_MINGW_PATH/include
LDFLAGS=-L$HOME_MINGW_PATH/lib
LIBS="-lm""

rm -Rf $BUILD_DIR

if ! [ -d $HOME_MINGW_PATH ] ; then
    mkdir $HOME_MINGW_PATH
    mkdir $HOME_MINGW_PATH/include
fi

JOBS=2

mkdir $BUILD_DIR
cd $BUILD_DIR

# zlib
if ! [ -f $HOME_MINGW_PATH/include/zlib.h ] ; then
    ZLIB_VERSION=1.2.7
    wget --continue \
        "http://sourceforge.net/projects/libpng/files/zlib/$ZLIB_VERSION/zlib-$ZLIB_VERSION.tar.gz/download" \
        -O zlib-$ZLIB_VERSION.tar.gz
    tar --overwrite -xf zlib-$ZLIB_VERSION.tar.gz
    cd zlib-$ZLIB_VERSION
    ./configure --prefix=$HOME_MINGW_PATH --static
    make libz.a && make install
    cd $BUILD_DIR
fi

# png
if ! [ -f $HOME_MINGW_PATH/include/png.h ] ; then
    PNG_VERSION=1.5.13
    wget --continue \
        "http://sourceforge.net/projects/libpng/files/libpng15/$PNG_VERSION/libpng-$PNG_VERSION.tar.gz/download" \
        -O libpng-$PNG_VERSION.tar.gz
    tar --overwrite -xf libpng-$PNG_VERSION.tar.gz
    cd libpng-$PNG_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# expat
if ! [ -f $HOME_MINGW_PATH/include/expat.h ] ; then
    EXPAT_VERSION=2.1.0
    wget --continue \
        "http://sourceforge.net/projects/expat/files/expat/$EXPAT_VERSION/expat-$EXPAT_VERSION.tar.gz/download" \
        -O expat-$EXPAT_VERSION.tar.gz
    tar --overwrite -xf expat-$EXPAT_VERSION.tar.gz
    cd expat-$EXPAT_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# jpeg
if ! [ -f $HOME_MINGW_PATH/include/jpeglib.h ] ; then
    wget --continue \
        "http://www.ijg.org/files/jpegsrc.v8d.tar.gz" \
        -O jpegsrc.v8d.tar.gz
    tar --overwrite -xf jpegsrc.v8d.tar.gz
    cd jpeg-8d
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# SDL
if ! [ -f $HOME_MINGW_PATH/include/SDL/SDL.h ] ; then
    SDL_VERSION=1.2.15
    wget --continue \
        "http://www.libsdl.org/release/SDL-$SDL_VERSION.tar.gz" \
        -O SDL-$SDL_VERSION.tar.gz
    tar --overwrite -xf SDL-$SDL_VERSION.tar.gz
    cd SDL-$SDL_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# libogg
if ! [ -f $HOME_MINGW_PATH/include/ogg/ogg.h ] ; then
    OGG_VERSION=1.3.0
    wget --continue \
        "http://downloads.xiph.org/releases/ogg/libogg-$OGG_VERSION.tar.gz" \
        -O libogg-$OGG_VERSION.tar.gz
    tar --overwrite -xf libogg-$OGG_VERSION.tar.gz
    cd libogg-$OGG_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

<<SDL_MIXER_THINGS
# libmikmod
if ! [ -f $HOME_MINGW_PATH/mikmod.h ] ; then
    MIKMOD_VERSION=3.2.2
    wget --continue \
        "http://mikmod.shlomifish.org/files/mikmod-$MIKMOD_VERSION.tar.gz" \
        -O libmikmod-$MIKMOD_VERSION.tar.gz
    tar --overwrite -xf libmikmod-$MIKMOD_VERSION.tar.gz
    cd mikmod-$MIKMOD_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# flac
if ! [ -f $HOME_MINGW_PATH/FLAC/all.h ] ; then
    FLAC_VERSION=1.2.1
    wget --continue \
        "http://sourceforge.net/projects/flac/files/flac-src/flac-$FLAC_VERSION-src/flac-$FLAC_VERSION.tar.gz/download" \
        -O flac-$FLAC_VERSION.tar.gz
    tar --overwrite -xf flac-$FLAC_VERSION.tar.gz
    cd flac-$FLAC_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# libvorbis
if ! [ -f $HOME_MINGW_PATH/include/vorbis/vorbisfile.h ] ; then
    VORBIS_VERSION=1.3.3
    wget --continue \
        "http://downloads.xiph.org/releases/vorbis/libvorbis-$VORBIS_VERSION.tar.gz" \
        -O libvorbis-$VORBIS_VERSION.tar.gz
    tar --overwrite -xf libvorbis-$VORBIS_VERSION.tar.gz
    cd libvorbis-$VORBIS_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# smpeg
if ! [ -f $HOME_MINGW_PATH/include/smpeg/smpeg.h ] ; then
    svn checkout "svn://svn.icculus.org/smpeg/trunk" smpeg
    cd smpeg
    ./autogen.sh
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# sdl_mixer
if ! [ -f $HOME_MINGW_PATH/include/SDL/SDL_mixer.h ] ; then
    SDL_MIXER_VERSION=1.2.12
    wget --continue \
        "http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-$SDL_MIXER_VERSION.tar.gz" \
        -O SDL_mixer-$SDL_MIXER_VERSION.tar.gz
    tar --overwrite -xf SDL_mixer-$SDL_MIXER_VERSION.tar.gz
    cd SDL_mixer-$SDL_MIXER_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi
SDL_MIXER_THINGS

# Freetype
if ! [ -f $HOME_MINGW_PATH/include/ft2build.h ] ; then
    FREETYPE_VERSION=2.4.10
    wget --continue \
        "http://download.savannah.gnu.org/releases/freetype/freetype-$FREETYPE_VERSION.tar.gz" \
        -O freetype-$FREETYPE_VERSION.tar.gz
    tar --overwrite -xf freetype-$FREETYPE_VERSION.tar.gz
    cd freetype-$FREETYPE_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# pixman
if ! [ -f $HOME_MINGW_PATH/include/pixman-1/pixman.h ] ; then
    PIXMAN_VERSION=0.26.2
    wget --continue \
        "http://cairographics.org/releases/pixman-$PIXMAN_VERSION.tar.gz" \
        -O pixman-$PIXMAN_VERSION.tar.gz
    tar --overwrite -xf pixman-$PIXMAN_VERSION.tar.gz
    cd pixman-$PIXMAN_VERSION
    sed -i -e "s/SUBDIRS = pixman demos test/SUBDIRS = pixman/" Makefile.in

    unset CC
    unset CXX
    unset RANLIB

    ./configure $CONFIGURE_FLAGS
    make -j $JOBS && make install
    cd $BUILD_DIR
fi

# boost
if ! [ -d $HOME_MINGW_PATH/include/boost ] ; then
    ln -s /usr/include/boost $HOME_MINGW_PATH/include/boost
    cd $BUILD_DIR
fi

make -C $SCRIPT_DIR

cd $START_DIR
