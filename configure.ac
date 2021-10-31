#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([easyrpg-player],[0.7.0],[https://github.com/EasyRPG/Player/issues],[easyrpg-player],[https://easyrpg.org])

AC_CONFIG_AUX_DIR([builds/autoconf/aux])
AM_INIT_AUTOMAKE([1.11.4 foreign subdir-objects tar-ustar -Wall dist-xz])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([builds/autoconf/m4])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AM_PROG_AR
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG
AC_PROG_OBJCXX

# Options
AC_ARG_ENABLE([fmmidi],
	AS_HELP_STRING([--enable-fmmidi],[use internal MIDI sequencer @<:@default=no@:>@]))
AS_IF([test "x$enable_fmmidi" = "xyes" -o "x$enable_fmmidi" = "xfallback"], [
	enable_fmmidi="yes" dnl fallback counts as yes, since it is default now
	AC_DEFINE([WANT_FMMIDI],[1],[Enable internal MIDI sequencer])
],[enable_fmmidi="no"])
AM_CONDITIONAL([WANT_FMMIDI],[test "x$enable_fmmidi" = "xyes"])

AC_ARG_ENABLE([drwav],
	AS_HELP_STRING([--disable-drwav],[use internal WAV library dr_wav @<:@default=yes@:>@]), ,[enable_drwav="yes"])
AS_IF([test "x$enable_drwav" = "xyes"],
	[AC_DEFINE([WANT_DRWAV],[1],[use internal WAV library dr_wav])],
	[enable_drwav="no"])
AM_CONDITIONAL([WANT_DRWAV],[test "x$enable_drwav" = "xyes"])

# Checks for libraries.
AC_DEFUN([EP_PKG_CHECK],[ dnl VARIABLE-PREFIX, MODULES, [DESCRIPTION], [DEFAULT=auto]
	PKG_WITH_MODULES([$1],[$2],,,[$3],[$4])
	AS_IF([test "$AS_TR_SH([with_]m4_tolower([$1]))" = "yes"],[
		AC_DEFINE([HAVE_][$1], [1], [$3])
	])
])

PKG_CHECK_MODULES([LCF],[liblcf])
PKG_CHECK_MODULES([PIXMAN],[pixman-1])
PKG_CHECK_MODULES([ZLIB],[zlib])
PKG_CHECK_MODULES([PNG],[libpng])
PKG_CHECK_MODULES([FMT],[fmt],,[
	AC_CHECK_HEADER([fmt/core.h],[fmtlib_header=1],,[ ])
	AC_MSG_CHECKING([for FMT (legacy)])
	saved_ldflags="${LDFLAGS}"
	LDFLAGS="${LDFLAGS} -lfmt"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <fmt/core.h>],[fmt::format("")])],[FMT_LIBS="-lfmt"])
	LDFLAGS="${saved_ldflags}"
	AS_IF([test -z "$FMT_LIBS" -o -z "$fmtlib_header"],[AC_MSG_RESULT([no])
		AC_MSG_ERROR([Could not find libfmt! Consider installing version 5.3 or newer.])
	],[AC_MSG_RESULT([yes])])
])
PKG_CHECK_MODULES([SDL],[sdl2 >= 2.0.5],[AC_DEFINE(USE_SDL,[2],[Enable SDL2])],[
	PKG_CHECK_MODULES([SDL],[sdl],[AC_DEFINE(USE_SDL,[1],[Enable SDL])])
])
EP_PKG_CHECK([FREETYPE],[freetype2],[Custom Font rendering.])
AS_IF([test "$with_freetype" = "yes"],[
	EP_PKG_CHECK([HARFBUZZ],[harfbuzz],[Custom Font text shaping.])
])

AC_ARG_WITH([audio],[AS_HELP_STRING([--without-audio], [Disable audio support. @<:@default=on@:>@])])
AS_IF([test "x$with_audio" != "xno"],[
	AC_DEFINE(SUPPORT_AUDIO,[1],[Enable Audio Support])
	EP_PKG_CHECK([LIBMPG123],[libmpg123],[MP3 support.])
	EP_PKG_CHECK([LIBWILDMIDI],[wildmidi],[Midi support (GUS patches). Alternative to internal fmmidi.])
	EP_PKG_CHECK([FLUIDSYNTH],[fluidsynth],[Midi support (Soundfonts). Alternative to internal fmmidi.])
	AS_IF([test "$with_fluidsynth" != "yes"],[
		EP_PKG_CHECK([FLUIDLITE],[fluidlite],[Midi support (Soundfonts). Liteweight version of FluidSynth. Alternative to internal fmmidi.])
	], [AC_SUBST([with_fluidlite], "no"]))
	EP_PKG_CHECK([ALSA],[alsa],[Midi support (Daemon)])
	EP_PKG_CHECK([OGGVORBIS],[vorbisfile vorbis],[Ogg Vorbis support.])
	EP_PKG_CHECK([OPUS],[opusfile],[Opus support.])
	EP_PKG_CHECK([LIBSNDFILE],[sndfile],[Improved WAV support. Fallback when unsupported by dr_wav.])
	EP_PKG_CHECK([LIBXMP],[libxmp >= 4.5.0],[Tracker module support.])
	EP_PKG_CHECK([LIBSPEEXDSP],[speexdsp],[Resampling support.])

	AS_IF([test "$with_alsa" = "yes"],[
		AC_DEFINE(HAVE_NATIVE_MIDI,[1],[Native Midi support])
		AX_PTHREAD
	])
])
AM_CONDITIONAL([HAVE_ALSA], [test "$with_alsa" = "yes"])

# bash completion
AC_ARG_WITH([bash-completion-dir],[AS_HELP_STRING([--with-bash-completion-dir@<:@=DIR@:>@],
	[Install the parameter auto-completion script for bash in DIR. @<:@default=auto@:>@])],
	[],[with_bash_completion_dir=yes])
AS_IF([test "x$with_bash_completion_dir" = "xyes"],[
	BASHCOMPLETION_DIR="`$PKG_CONFIG --silence-errors --define-variable=prefix="\${prefix}" --define-variable=datadir="\${datadir}" --variable=completionsdir bash-completion`"
],[
	BASHCOMPLETION_DIR=$with_bash_completion_dir
])
AS_IF([test "x$BASHCOMPLETION_DIR" = "x"],[
	BASHCOMPLETION_DIR="${datadir}/bash-completion/completions"
])
AC_SUBST([BASHCOMPLETION_DIR])
AM_CONDITIONAL([HAVE_BASHCOMPLETION], [test "x$with_bash_completion_dir" != "xno"])

# Expose OS type to automake
AC_CANONICAL_HOST

EP_BUILD_MAC=NO

# Detect the target system
case "${host_os}" in
    darwin*)
        EP_BUILD_MAC=YES
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([MACOS], [test "$EP_BUILD_MAC" = "YES"])

# C++14 is mandatory
AX_CXX_COMPILE_STDCXX(14, noext)

# Checks for header files.
AC_CHECK_HEADERS([cstdint cstdlib string iostream unistd.h wchar.h])

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
AC_CHECK_FUNCS([malloc floor getcwd memset putenv strerror])

# manual page
AC_CHECK_PROGS([A2X], [a2x a2x.py], [no])
AM_CONDITIONAL([HAVE_A2X], [test x"$A2X" != "xno"])
AC_SUBST([A2X])
AS_IF([test x"$A2X" = "xno" && test ! -f "${srcdir}/resources/easyrpg-player.6"],
  AC_MSG_WARN([a2x is required to create the manual page]))
AM_CONDITIONAL([HAVE_MANUAL], [test -f "${srcdir}/resources/easyrpg-player.6"])

# Doxygen source documentation
m4_include([builds/autoconf/m4/ax_prog_doxygen.m4])
DX_DOXYGEN_FEATURE(OFF)
DX_HTML_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_INIT_DOXYGEN(easyrpg-player, resources/Doxyfile, doc)

# Files to generate
AC_CONFIG_FILES([Makefile
                 resources/Doxyfile])

# Absolute & Canonical source directory for unit tests
canonical_srcdir=`cd $srcdir && pwd -P`
AC_SUBST([canonical_srcdir])

AC_OUTPUT

if test "yes" != "$silent"; then
	echo "==============================================================================="

	echo "EasyRPG Player $VERSION configuration summary:"
	echo ""

	echo "Paths:"
	echo "  prefix: $prefix"
	echo "  bash completion: $BASHCOMPLETION_DIR"

	# FIXME SDL version
	echo "Backend: SDL"

	echo "Optional features:"
	echo "  -custom Font rendering (freetype2):   $with_freetype"
	test "$with_freetype" = "yes" && \
		echo "  -custom Font text shaping (harfbuzz): $with_harfbuzz"

	if test "$with_audio" = "no"; then
		echo "Audio support:               no"
	else
		echo "Audio support:"
		echo "  -FMMidi:                   $enable_fmmidi"
		echo "  -MP3 (libmpg123):          $with_libmpg123"
		echo "  -midi (libwildmidi):       $with_libwildmidi"
		echo "  -midi (FluidSynth):        $with_fluidsynth"
		echo "  -midi (FluidLite):         $with_fluidlite"
		echo "  -midi (Native, ALSA):      $with_alsa"
		echo "  -ogg Vorbis (vorbisfile):  $with_oggvorbis"
		echo "  -opus (opusfile):          $with_opus"
		echo "  -built-in WAV (dr_wav):    $enable_drwav"
		echo "  -WAV (sndfile):            $with_libsndfile"
		echo "  -tracker module (libxmp):  $with_libxmp"
		echo "  -resampling (speexdsp):    $with_libspeexdsp"
	fi

	echo "Documentation:"
	echo -n "  manual page: "
	if test "$A2X" = "no" -a -f "${srcdir}/resources/easyrpg-player.6"; then
		echo "shipped"
	elif test "x$A2X" != "xno"; then
		echo "generated"
	else
		echo "not available"
	fi
	echo -n "  doxygen:     "
	test "$DX_FLAG_doc" = 1 && echo "yes" || echo "no"

	echo "==============================================================================="
fi
