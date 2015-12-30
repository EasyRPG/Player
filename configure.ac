#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([easyrpg-player],[0.4.0],[https://github.com/EasyRPG/Player/issues],[easyrpg-player],[https://easy-rpg.org/])

AC_CONFIG_AUX_DIR([builds/autoconf])
AM_INIT_AUTOMAKE([1.11.4 foreign subdir-objects -Wall -Werror])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([builds/autoconf/m4])
AC_CONFIG_SRCDIR([src/battle_animation.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AM_PROG_AR
AC_PROG_CXX
LT_INIT([disable-shared])

# Checks for libraries.
PKG_CHECK_MODULES([LCF],[liblcf = 0.4.0])
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


AC_EGREP_CPP(yes,
[#ifdef EMSCRIPTEN
  yes
#endif
],[],[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([boost/foreach.hpp],[],[AC_MSG_ERROR(['boost' is required but it doesn't seem to be installed on this system.])])
AC_LANG_POP([C++])])

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

# manual page
AC_CHECK_PROGS([A2X], [a2x a2x.py], [no])
AM_CONDITIONAL([HAVE_A2X], [test x"$A2X" != "xno"])
AC_SUBST([A2X])
AS_IF([test x"$A2X" = "xno" && test ! -f "${srcdir}/builds/easyrpg-player.6"],
  AC_MSG_WARN([a2x is required to create the manual page]))
AM_CONDITIONAL([HAVE_MANUAL], [test -f "${srcdir}/builds/easyrpg-player.6"])

# Doxygen source documentation
m4_include([builds/autoconf/m4/ax_prog_doxygen.m4])
DX_DOXYGEN_FEATURE(OFF)
DX_HTML_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_INIT_DOXYGEN(easyrpg-player, builds/Doxyfile, doc)

AC_CONFIG_FILES([Makefile
                 builds/Doxyfile])

AC_OUTPUT
