#!/bin/bash -e

export START_DIR=$(pwd)

export BUILD_TARGET="i686-w64-mingw32"

cd $(dirname $0)
export SCRIPT_DIR=$(pwd)
export BUILD_DIR=$SCRIPT_DIR/build

export HOME_BUILD_PATH="$SCRIPT_DIR/mingw"

export SYSROOT="/"
export   CFLAGS=-I$HOME_BUILD_PATH/include
export CXXFLAGS=-I$HOME_BUILD_PATH/include
export  LDFLAGS="-L$HOME_BUILD_PATH/lib -lm"

export CONFIGURE_FLAGS="--host=$BUILD_TARGET
--prefix=$HOME_BUILD_PATH
--enable-shared=no
--enable-static=yes"

export CMAKE_SYSTEM_NAME="Windows"
export JOBS=2

export BOOST_OPTIONS="threadapi=pthread target-os=windows --without-serialization"
export PREFIX_PATHS="$MINGW_PATH"

source $SCRIPT_DIR/../pre_build.sh

# winpthreads
if ! [ -f $HOME_BUILD_PATH/include/pthread.h ] ; then
    svn checkout "https://mingw-w64.svn.sourceforge.net/svnroot/mingw-w64/experimental/winpthreads"
    cd winpthreads
    copy_config
    ./configure $CONFIGURE_FLAGS
    make
    make install
fi

# SDL
if ! [ -f $HOME_BUILD_PATH/include/SDL/SDL.h ] ; then
    SDL_VERSION=1.2.15
    wget --continue \
        "http://www.libsdl.org/release/SDL-$SDL_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/SDL-$SDL_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/SDL-$SDL_VERSION.tar.gz
    cd SDL-$SDL_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

source $SCRIPT_DIR/../build_libraries.sh

make -C $SCRIPT_DIR -j $JOBS

cd $START_DIR
