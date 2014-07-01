#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([easyrpg-player],[0.2.1],[https://github.com/EasyRPG/Player/issues],[easyrpg-player],[https://easy-rpg.org/])

AC_CONFIG_AUX_DIR([builds/autoconf])
AM_INIT_AUTOMAKE([1.11.4 foreign subdir-objects -Wall -Werror])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/battle_animation.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES([LCF],[liblcf])
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
AC_LANG_POP([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
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
