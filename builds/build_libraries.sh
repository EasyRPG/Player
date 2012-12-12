#!/bin/bash -e

# iconv
if ! [ -f $HOME_BUILD_PATH/include/iconv.h ] ; then
    ICONV_VERSION=1.14
    wget --continue \
        "http://ftp.gnu.org/pub/gnu/libiconv/libiconv-$ICONV_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/libiconv-$ICONV_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/libiconv-$ICONV_VERSION.tar.gz
    cd libiconv-$ICONV_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install

    cd $BUILD_DIR
fi

# bzip2
if ! [ -f $HOME_BUILD_PATH/include/bzlib.h ] ; then
    BZIP2_VERSION=1.0.6
    wget --continue \
        "http://www.bzip.org/1.0.6/bzip2-$BZIP2_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/bzip2-$BZIP2_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/bzip2-$BZIP2_VERSION.tar.gz
    cd bzip2-$BZIP2_VERSION
    copy_config

    sed -i -e "s/sys.stat.h/sys\/stat.h/" bzip2.c
    sed -i -e "s/all: libbz2.a bzip2 bzip2recover test/all: libbz2.a bzip2 bzip2recover/" Makefile

    make -j $JOBS \
        CC="$CC $CFLAGS" AR=$AR RANLIB=$RANLIB \
        PREFIX=$HOME_BUILD_PATH install

    cd $BUILD_DIR
fi

# zlib
if ! [ -f $HOME_BUILD_PATH/include/zlib.h ] ; then
    ZLIB_VERSION=1.2.7
    wget --continue \
        "http://zlib.net/zlib-$ZLIB_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/zlib-$ZLIB_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/zlib-$ZLIB_VERSION.tar.gz
    cd zlib-$ZLIB_VERSION
    copy_config
    ./configure --prefix=$HOME_BUILD_PATH --static
    make AR="$AR" ARFLAGS="rvs" RANLIB="$RANLIB" libz.a
	  make AR="$AR" ARFLAGS="rvs" RANLIB="$RANLIB" install

    cd $BUILD_DIR
fi

# png
if ! [ -f $HOME_BUILD_PATH/include/png.h ] ; then
    PNG_VERSION=1.5.13
    wget --continue \
        "ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-$PNG_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/libpng-$PNG_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/libpng-$PNG_VERSION.tar.gz
    cd libpng-$PNG_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install

    cd $BUILD_DIR
fi

# expat
if ! [ -f $HOME_BUILD_PATH/include/expat.h ] ; then
    EXPAT_VERSION=2.1.0
    wget --continue \
        "http://sourceforge.net/projects/expat/files/expat/$EXPAT_VERSION/expat-$EXPAT_VERSION.tar.gz/download" \
        -O $ARCHIVE_DIR/expat-$EXPAT_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/expat-$EXPAT_VERSION.tar.gz
    cd expat-$EXPAT_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# jpeg
if ! [ -f $HOME_BUILD_PATH/include/jpeglib.h ] ; then
    wget --continue \
        "http://www.ijg.org/files/jpegsrc.v8d.tar.gz" \
        -O $ARCHIVE_DIR/jpegsrc.v8d.tar.gz
    tar -xf $ARCHIVE_DIR/jpegsrc.v8d.tar.gz
    cd jpeg-8d
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# libogg
if ! [ -f $HOME_BUILD_PATH/include/ogg/ogg.h ] ; then
    OGG_VERSION=1.3.0
    wget --continue \
        "http://downloads.xiph.org/releases/ogg/libogg-$OGG_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/libogg-$OGG_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/libogg-$OGG_VERSION.tar.gz
    cd libogg-$OGG_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# libvorbis
if ! [ -f $HOME_BUILD_PATH/include/vorbis/vorbisfile.h ] ; then
    VORBIS_VERSION=1.3.3
    wget --continue \
        "http://downloads.xiph.org/releases/vorbis/libvorbis-$VORBIS_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/libvorbis-$VORBIS_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/libvorbis-$VORBIS_VERSION.tar.gz
    cd libvorbis-$VORBIS_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# FLAC
if ! [ -f $HOME_BUILD_PATH/include/FLAC/all.h ] ; then
    FLAC_VERSION=1.2.1
    wget --continue \
        "http://sourceforge.net/projects/flac/files/flac-src/flac-$FLAC_VERSION-src/flac-$FLAC_VERSION.tar.gz/download" \
        -O $ARCHIVE_DIR/flac-$FLAC_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/flac-$FLAC_VERSION.tar.gz
    cd flac-$FLAC_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    sed -i -e 's/#include <stdio.h>/#include <stdio.h>\
#include<string.h>/' examples/cpp/encode/file/main.cpp
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# libsndfile
if ! [ -f $HOME_BUILD_PATH/include/sndfile.h ] ; then
    SNDFILE_VERSION=1.0.25
    wget --continue \
        "http://www.mega-nerd.com/libsndfile/files/libsndfile-$SNDFILE_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/libsndfile-$SNDFILE_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/libsndfile-$SNDFILE_VERSION.tar.gz
    cd libsndfile-$SNDFILE_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS --disable-sqlite --disable-alsa
    cd src
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# Freetype
if ! [ -f $HOME_BUILD_PATH/include/ft2build.h ] ; then
    FREETYPE_VERSION=2.4.10
    wget --continue \
        "http://download.savannah.gnu.org/releases/freetype/freetype-$FREETYPE_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/freetype-$FREETYPE_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/freetype-$FREETYPE_VERSION.tar.gz
    cd freetype-$FREETYPE_VERSION
    copy_config
    ./configure $CONFIGURE_FLAGS
    make -j $JOBS
	  make install
    cd $BUILD_DIR
fi

# pixman
if ! [ -f $HOME_BUILD_PATH/include/pixman-1/pixman.h ] ; then
    PIXMAN_VERSION=0.28.2
    wget --continue \
        "http://cairographics.org/releases/pixman-$PIXMAN_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/pixman-$PIXMAN_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/pixman-$PIXMAN_VERSION.tar.gz
    cd pixman-$PIXMAN_VERSION
    copy_config

    unset CC
    unset CXX
    unset RANLIB

    CFLAGS="$CFLAGS -DSIZE_MAX=\"((size_t)-1)\"" ./configure $CONFIGURE_FLAGS
    make SUBDIRS=pixman -j $JOBS
	  make SUBDIRS=pixman install
    cd $BUILD_DIR
fi

# Lua
if ! [ -f $HOME_BUILD_PATH/include/lua.h ] ; then
    LUA_VERSION=5.1.5
    wget --continue \
        "http://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz" \
        -O $ARCHIVE_DIR/lua-$LUA_VERSION.tar.gz
    tar -xf $ARCHIVE_DIR/lua-$LUA_VERSION.tar.gz
    cd lua-$LUA_VERSION
    copy_config
    sed -i -e "s/cv->decimal_point\[0\]/'.'/" src/llex.c
    sed -i -e "s/localeconv()/NULL/" src/llex.c
    make -j $JOBS \
        CC="$CC" AR="$AR rvs" RANLIB="$RANLIB" \
        MYCFLAGS="$LUA_CFLAGS" MYLDFLAGS="$LUA_LDFLAGS" generic
	  make INSTALL_TOP=$HOME_BUILD_PATH TO_BIN="$LUA_EXECUTABLE" install

    cd $BUILD_DIR
fi

# OpenAL Soft
if ! [ -f $HOME_BUILD_PATH/include/AL/AL.h ] ; then
    OPENAL_VERSION=1.15.1
    wget --continue \
        "http://kcat.strangesoft.net/openal-releases/openal-soft-$OPENAL_VERSION.tar.bz2" \
        -O $ARCHIVE_DIR/openal-soft-$OPENAL_VERSION.tar.bz2
    tar -xf $ARCHIVE_DIR/openal-soft-$OPENAL_VERSION.tar.bz2
    cd openal-soft-$OPENAL_VERSION

    sed -i -e "s/\(case SL_RESULT_READONLY\)/\/\/ \1/" Alc/backends/opensl.c
    sed -i -e "s/\(case SL_RESULT_ENGINEOPTION_UNSUPPORTED\)/\/\/ \1/" Alc/backends/opensl.c
    sed -i -e "s/\(case SL_RESULT_SOURCE_SINK_INCOMPATIBLE\)/\/\/ \1/" Alc/backends/opensl.c
    sed -i -e "s/SL_BYTEORDER_NATIVE/IS_LITTLE_ENDIAN? SL_BYTEORDER_LITTLEENDIAN : SL_BYTEORDER_BIGENDIAN/" Alc/backends/opensl.c

    cmake \
        -DCMAKE_BUILD_TYPE=DEBUG \
        -DCMAKE_TOOLCHAIN_FILE=$SCRIPT_DIR/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=$HOME_BUILD_PATH \
        -DLIBTYPE=STATIC \
        -DEXAMPLES=FALSE \
        -DUTILS=FALSE \
        .

    make -j $JOBS
    make install

    cd $BUILD_DIR
fi

# boost
if ! [ -f $HOME_BUILD_PATH/include/boost/shared_ptr.hpp ] ; then
    BOOST_VERSION=1.52.0
    BOOST_VERSION_UNDER=$(echo $BOOST_VERSION | sed -e "s/\./_/g")
    wget --continue --no-check-certificate \
        "https://github.com/ryppl/boost-svn/archive/Boost_$BOOST_VERSION_UNDER.zip" \
        -O $ARCHIVE_DIR/boost-$BOOST_VERSION_UNDER.tar.bz2
    tar -xf $ARCHIVE_DIR/boost-$BOOST_VERSION_UNDER.tar.bz2
    cd boost-svn-Boost_$BOOST_VERSION_UNDER
    copy_config

    ./bootstrap.sh gcc

    # endian fix
    sed -i -e 's/\(#else\)/#elif defined(_BYTE_ORDER)\
#  if (_BYTE_ORDER == _BIG_ENDIAN)\
#    define BOOST_BIG_ENDIAN\
#    define BOOST_BYTE_ORDER 4321\
#  elif (_BYTE_ORDER == _LITTLE_ENDIAN)\
#    define BOOST_BIG_ENDIAN\
#    define BOOST_BYTE_ORDER 4321\
#  endif\
\1/' boost/detail/endian.hpp

    # statvfs fix
    sed -i -e \
        's/^\(#   if !defined(__APPLE__) \&\& !defined(__OpenBSD__)\)$/\1 \&\& !defined(ANDROID)/' \
        libs/filesystem/src/operations.cpp
    sed -i -e 's/^\(#     define BOOST_STATVFS_F_FRSIZE vfs.f_frsize\)/\1\
#   elif defined (ANDROID)\
#     include <sys\/vfs.h>\
#     define BOOST_STATVFS statfs\
#     define BOOST_STATVFS_F_FRSIZE static_cast<boost::uintmax_t>(vfs.f_bsize)/' \
    libs/filesystem/src/operations.cpp

    # PAGE_SIZE fix
    sed -i -e 's/\(#include <pthread.h>\)/\
#define PAGE_SIZE sysconf(_SC_PAGESIZE)\
\1/' boost/thread/pthread/thread_data.hpp

    echo "using gcc : $($BUILD_TARGET-g++ -dumpversion) : ${CCACHE} $BUILD_TARGET-g++ : -std=gnu++0x ;" > user-config.jam

    ./bjam toolset=gcc $BOOST_OPTIONS \
        --prefix=$HOME_BUILD_PATH \
        --without-python --without-mpi \
        --without-context \
        --build-type=complete --layout=tagged \
        --user-config=user-config.jam \
        include=$HOME_BUILD_PATH/include \
        library-path=$HOME_BUILD_PATH/lib \
        variant=debug,release \
        threading=multi \
        link=static runtime-link=static \
        cxxflags="$CXXFLAGS" cflags="$CFLAGS" linkflags="$LDFLAGS" \
        release debug stage install -j $JOBS

    cd $BUILD_DIR
fi
