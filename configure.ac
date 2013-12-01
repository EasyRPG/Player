#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([easyrpg-player],[0.1.3],[https://github.com/EasyRPG/Player/issues])

AM_INIT_AUTOMAKE([1.11 foreign subdir-objects -Wall -Werror -Wno-extra-portability])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/battle_animation.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
m4_ifdef([PKG_CHECK_MODULES],[],[AC_MSG_ERROR['pkg-config' is required but it doesn't seem to be installed on this system (PKG_CHECK_MODULES macro not found).]])

# Checks for libraries.
PKG_CHECK_MODULES([PIXMAN],[pixman-1],[CXXFLAGS="$CXXFLAGS $PIXMAN_CFLAGS" LIBS="$LIBS $PIXMAN_LIBS"])
PKG_CHECK_MODULES([FREETYPE],[freetype2],[CXXFLAGS="$CXXFLAGS $FREETYPE_CFLAGS" LIBS="$LIBS $FREETYPE_LIBS"])
m4_ifdef([AM_PATH_SDL],[AM_PATH_SDL([1.2.0],[CXXFLAGS="$CXXFLAGS -DUSE_SDL $SDL_CFLAGS" LIBS="$LIBS $SDL_LIBS"])],[AC_MSG_ERROR['SDL' is required but it doesn't seem to be installed on this system (AM_PATH_SDL macro not found).]])
AC_CHECK_LIB([SDL_mixer],[Mix_OpenAudio],[CXXFLAGS="$CXXFLAGS -DHAVE_SDL_MIXER" LIBS="$LIBS -lSDL_mixer"],[AC_MSG_ERROR(['SDL_mixer' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_LIB([png],[png_set_filler],[],[AC_MSG_ERROR(['libpng' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_LIB([z],[uncompress],[],[AC_MSG_ERROR(['zlib' is required but it doesn't seem to be installed on this system.])])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h wchar.h])
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([boost/foreach.hpp],[],[AC_MSG_ERROR(['boost' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_HEADERS([reader_struct.h],[],[AC_MSG_ERROR([EasyRPG 'readers' library is required but it doesn't seem to be installed on this system.])])
AC_LANG_POP([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor getcwd memset putenv strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
