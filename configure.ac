#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([easyrpg-player], [0.1], [https://github.com/EasyRPG/Player/issues])

AM_INIT_AUTOMAKE([1.11])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AM_PATH_SDL([1.2.0],[CXXFLAGS="$CXXFLAGS -DUSE_SDL $SDL_CFLAGS"])
AC_CHECK_LIB([SDL_mixer],[Mix_OpenAudio])
AC_CHECK_LIB([SDL_image],[IMG_Load])
AC_CHECK_LIB([SDL_ttf], [TTF_OpenFont])
AC_CHECK_LIB([z],[uncompress])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset putenv strerror])

AC_ARG_WITH([readers],[  --with-readers=DIR      location of the readers library],
	[if test "$withval" != no; then
		if test "$withval" != yes; then
			READERS_DIR=$withval
		fi
	fi])

if test "$withval" != no; then
	if test -n "$READERS_DIR"; then
		CXXFLAGS="$CXXFLAGS -I$READERS_DIR/include"
	fi
fi

AC_SUBST(READERS_DIR)

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
