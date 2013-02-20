#!/bin/bash -e

rm -Rf $BUILD_DIR

if ! [ -d $HOME_ANDROID_PATH ] ; then
    mkdir $HOME_ANDROID_PATH
    mkdir $HOME_ANDROID_PATH/include
fi

export ARCHIVE_DIR=$SCRIPT_DIR/../archive

if ! [ -d $ARCHIVE_DIR ] ; then
    mkdir $ARCHIVE_DIR
fi

ls $ARCHIVE_DIR/config.guess || wget --continue -O $ARCHIVE_DIR/config.guess \
    "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
ls $ARCHIVE_DIR/config.sub || wget --continue -O $ARCHIVE_DIR/config.sub \
    "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"

    (cat <<EOF

set(CMAKE_SYSTEM_NAME $CMAKE_SYSTEM_NAME)

include(CMakeForceCompiler)
cmake_force_c_compiler($BUILD_TARGET-gcc "GNU")
cmake_force_cxx_compiler($BUILD_TARGET-g++ "GNU")

message(STATUS "C Compiler: \${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: \${CMAKE_CXX_COMPILER}")

set(CMAKE_CROSSCOMPILING True)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

list(APPEND CMAKE_C_FLAGS "$CFLAGS")
list(APPEND CMAKE_CXX_FLAGS "$CXXFLAGS")

list(APPEND CMAKE_PREFIX_PATH "$HOME_BUILD_PATH" $PREFIX_PATHS "$SYSROOT/usr")

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(CMAKE_RC_COMPILER $BUILD_TARGET-windres)
  set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  set(CMAKE_SYSTEM_PROCESSOR "x86")
  set(CMAKE_HOST_WIN32 TRUE)
endif()

EOF
    ) > $SCRIPT_DIR/toolchain.cmake

copy_config()
{
    for i in $(find . -name config.sub)
    do
        cp $ARCHIVE_DIR/config.sub $i
    done
    for i in $(find . -name config.guess)
    do
        cp $ARCHIVE_DIR/config.guess $i
    done
}

export CCACHE="$(which ccache)"
export NDK_CCACHE=$CCACHE
echo "ccache: $CCACHE"

export CC="${CCACHE} $(which $BUILD_TARGET-gcc)"
export CXX="${CCACHE} $(which $BUILD_TARGET-g++)"
export CPP="$(which $BUILD_TARGET-cpp)"
export STRIP="$(which $BUILD_TARGET-strip)"
export NM="$(which $BUILD_TARGET-nm)"
export AR="$(which $BUILD_TARGET-ar)"
export LD="$(which $BUILD_TARGET-ld)"
export RANLIB="$(which $BUILD_TARGET-ranlib)"

mkdir $BUILD_DIR
cd $BUILD_DIR
