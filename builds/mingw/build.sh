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


# iconv
if ! [ -f $HOME_MINGW_PATH/include/iconv.h ] ; then
    ICONV_VERSION=1.14
    wget --continue \
        "http://ftp.gnu.org/pub/gnu/libiconv/libiconv-$ICONV_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/libiconv-$ICONV_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/libiconv-$ICONV_VERSION.tar.gz
    cd libiconv-$ICONV_VERSION
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install

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

# bzip2
if ! [ -f $HOME_MINGW_PATH/include/bzlib.h ] ; then
    BZIP2_VERSION=1.0.6
    wget --continue \
        "http://www.bzip.org/1.0.6/bzip2-$BZIP2_VERSION.tar.gz" \
        -O $SCRIPT_DIR/archive/bzip2-$BZIP2_VERSION.tar.gz
    tar -xf $SCRIPT_DIR/archive/bzip2-$BZIP2_VERSION.tar.gz
    cd bzip2-$BZIP2_VERSION

    sed -i -e "s/\$(PREFIX)\/bin\/\([a-z0-9A-Z_][a-z0-9A-Z_]*\)/\$(PREFIX)\/bin\/\1.exe/" Makefile
    sed -i -e "s/sys.stat.h/sys\/stat.h/" bzip2.c

    make -j $JOBS CC=$MINGW_TARGET-gcc AR=$MINGW_TARGET-ar RANLIB=$MINGW_TARGET-ranlib
	  make PREFIX=$HOME_MINGW_PATH install

    cd $BUILD_DIR
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
    sed -i -e "s/AR= ar/AR=$MINGW_TARGET-ar/" src/Makefile
    sed -i -e "s/RANLIB= ranlib/RANLIB=$MINGW_TARGET-ranlib/" src/Makefile
    sed -i -e "s/RANLIB=strip/RANLIB=$MINGW_TARGET-strip/" src/Makefile
    make -j $JOBS PLAT=mingw

	  make INSTALL_TOP=$HOME_MINGW_PATH \
        "TO_BIN=lua.exe luac.exe lua52.dll" \
        install

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
    BOOST_VERSION=1.52.0
    BOOST_VERSION_UNDER=$(echo $BOOST_VERSION | sed -e "s/\./_/g")
    wget --continue --max-redirect=50 \
        "http://sourceforge.net/projects/boost/files/boost/$BOOST_VERSION/boost-$BOOST_VERSION_UNDER.tar.bz2/download" \
        -O $SCRIPT_DIR/archive/boost-$BOOST_VERSION_UNDER.tar.bz2
    tar -xf $SCRIPT_DIR/archive/boost-$BOOST_VERSION_UNDER.tar.bz2
    cd boost_$BOOST_VERSION_UNDER

    ./bootstrap.sh gcc

    echo "using gcc : 4.6.3 : ccache i686-w64-mingw32-g++ -pipe : ;" > user-config.jam
    wget --continue --no-check-certificate \
        -O tools/build/v2/tools/mingw.jam \
        "https://svn.boost.org/trac/boost/raw-attachment/ticket/6350/mingw.jam"

    ./bjam target-os=windows toolset=gcc \
        --exec-prefix=$HOME_MINGW_PATH \
        --without-python --without-mpi --without-context \
        --build-type=complete --layout=tagged \
        --user-config=user-config.jam \
        include=$HOME_MINGW_PATH/include \
        library-path=$HOME_MINGW_PATH/lib \
        variant=debug,release \
        threading=multi threadapi=win32 \
        link=static runtime-link=static \
        release debug stage -j $JOBS

    cp -a boost $HOME_MINGW_PATH/include

    cd $BUILD_DIR
fi

make -C $SCRIPT_DIR

cd $START_DIR
