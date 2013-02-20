#!/bin/bash -e

export START_DIR=$(pwd)

export BUILD_TARGET="arm-linux-androideabi"

cd $(dirname $0)
export SCRIPT_DIR=$(pwd)
export BUILD_DIR=$SCRIPT_DIR/build

export HOME_BUILD_PATH="$SCRIPT_DIR/android"

export BUILD_PLATFORM_VERSION=9

export ANDROID_ABI=armeabi-v7a
export ANDROID_ARCH=arm

CXX_PREFIX=$NDK_PATH/sources/cxx-stl/gnu-libstdc++/$($BUILD_TARGET-g++ -dumpversion)

export SYSROOT="$NDK_PATH/platforms/android-$BUILD_PLATFORM_VERSION/arch-$ANDROID_ARCH"
export CPPFLAGS="--sysroot=$SYSROOT -I$HOME_BUILD_PATH/include -I$NDK_PATH/sources/cpufeatures -DANDROID=1"
export   CFLAGS="$CPPFLAGS"
export CXXFLAGS="$CPPFLAGS -I$CXX_PREFIX/include -I$CXX_PREFIX/libs/$ANDROID_ABI/include"
export  LDFLAGS="--sysroot=$SYSROOT -L$HOME_BUILD_PATH/lib -lm -lc -L$CXX_PREFIX/libs/$ANDROID_ABI -lgnustl_static -lsupc++"

export CONFIGURE_FLAGS="--host=$BUILD_TARGET
--prefix=$HOME_BUILD_PATH
--enable-shared=no
--enable-static=yes
gl_cv_header_working_stdint_h=yes
"

export CMAKE_SYSTEM_NAME="Linux"
export JOBS=2

export LUA_CFLAGS="-DDLUA_USE_POSIX $CFLAGS"
export LUA_LDFLAGS="$LDFLAGS"
export LUA_EXECUTABLE="lua luac"

export BOOST_OPTIONS="threadapi=pthread --without-locale"

source $SCRIPT_DIR/../pre_build.sh
source $SCRIPT_DIR/../build_libraries.sh

$NDK_PATH/ndk-build -j 2 -C $SCRIPT_DIR

cd $START_DIR
