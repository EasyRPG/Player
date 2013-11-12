#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([easyrpg-player],[0.1.3],[https://github.com/EasyRPG/Player/issues])

AM_INIT_AUTOMAKE([1.11 foreign subdir-objects -Wall -Werror -Wno-extra-portability])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/battle_animation.cpp])
AC_CONFIG_HEADERS([config.h])

CXXFLAGS="$CXXFLAGS -std=c++0x"

# Checks for programs.
AC_PROG_CXX
m4_ifdef([PKG_CHECK_MODULES],[],[AC_MSG_ERROR['pkg-config' is required but it doesn't seem to be installed on this system (PKG_CHECK_MODULES macro not found).]])

# Checks for libraries.
AC_ARG_WITH([readers],[AS_HELP_STRING([--with-readers=DIR],[Location of the EasyRPG 'readers' library])],[
	CPPFLAGS="$CPPFLAGS -I$withval/include"
	LDFLAGS="$LDFLAGS -L$withval/lib"
	AC_CHECK_LIB([readers],[ini_parse],[
		READERS_LIB="$withval/lib/libreaders.a"
	],[AC_MSG_ERROR([EasyRPG 'readers' library does not have been found in the specified path.])])
])
PKG_CHECK_MODULES([PIXMAN],[pixman-1],[CXXFLAGS="$CXXFLAGS $PIXMAN_CFLAGS" LIBS="$LIBS $PIXMAN_LIBS"])
PKG_CHECK_MODULES([FREETYPE],[freetype2],[CXXFLAGS="$CXXFLAGS $FREETYPE_CFLAGS" LIBS="$LIBS $FREETYPE_LIBS"])
m4_ifdef([AM_PATH_SDL],[AM_PATH_SDL([1.2.0],[CXXFLAGS="$CXXFLAGS -DUSE_SDL $SDL_CFLAGS" LIBS="$LIBS $SDL_LIBS"])],[AC_MSG_ERROR['SDL' is required but it doesn't seem to be installed on this system (AM_PATH_SDL macro not found).]])
AC_CHECK_LIB([SDL_mixer],[Mix_OpenAudio],[CXXFLAGS="$CXXFLAGS -DHAVE_SDL_MIXER" LIBS="$LIBS -lSDL_mixer"],[AC_MSG_ERROR(['SDL_mixer' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_LIB([png],[png_set_filler],[],[AC_MSG_ERROR(['libpng' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_LIB([z],[uncompress],[],[AC_MSG_ERROR(['zlib' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_LIB([readers],[ini_parse],[],[AC_MSG_ERROR([EasyRPG 'readers' library is required but does not have been found. If you have already compiled it, use ./configure --with-readers=DIR to specify its location.])])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h wchar.h])

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

AC_SUBST(READERS_LIB)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
