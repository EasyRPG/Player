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
PKG_CHECK_MODULES([PIXMAN],[pixman-1])
PKG_CHECK_MODULES([FREETYPE],[freetype2])
PKG_CHECK_MODULES([SDL],[sdl2],[AC_DEFINE(USE_SDL,[1],[Enable SDL2])],[
	PKG_CHECK_MODULES([SDL],[sdl],[AC_DEFINE(USE_SDL,[1],[Enable SDL])])
])
PKG_CHECK_MODULES([SDLMIXER],[SDL2_mixer],[AC_DEFINE(HAVE_SDL_MIXER,[1],[Enable SDL2_Mixer])],[
	PKG_CHECK_MODULES([SDLMIXER],[SDL_mixer],[AC_DEFINE(HAVE_SDL_MIXER,[1],[Enable SDL_Mixer])])
])
PKG_CHECK_MODULES([PNG],[libpng])
PKG_CHECK_MODULES([ZLIB],[zlib])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h wchar.h])
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([boost/foreach.hpp],[],[AC_MSG_ERROR(['boost' is required but it doesn't seem to be installed on this system.])])
AC_CHECK_HEADERS([reader_options.h],[],[AC_MSG_ERROR(['liblcf' is required but it doesn't seem to be installed on this system.])])
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
