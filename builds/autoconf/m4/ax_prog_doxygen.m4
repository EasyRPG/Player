# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_prog_doxygen.html
# ===========================================================================
#
# SYNOPSIS
#
#   DX_INIT_DOXYGEN(PROJECT-NAME, DOXYFILE-PATH, [OUTPUT-DIR])
#   DX_DOXYGEN_FEATURE(ON|OFF)
#   DX_DOT_FEATURE(ON|OFF)
#   DX_HTML_FEATURE(ON|OFF)
#
# DESCRIPTION
#
#   The DX_*_FEATURE macros control the default setting for the given
#   Doxygen feature. Supported features are 'DOXYGEN' itself, 'DOT' for
#   generating graphics, 'HTML' for plain HTML.
#
#   By default, HTML documentation is generated as this seems to be the most
#   popular and portable combination.
#
#   The macros mainly control the default state of the feature. The use can
#   override the default by specifying --enable or --disable. The macros
#   ensure that contradictory flags are not given (e.g.,
#   --enable-doxygen-anything with --disable-doxygen, etc.) Finally, each
#   feature will be automatically disabled (with a warning) if the required
#   programs are missing.
#
#   Once all the feature defaults have been specified, call DX_INIT_DOXYGEN
#   with the following parameters: a one-word name for the project for use
#   as a filename base etc., an optional configuration file name (the
#   default is 'Doxyfile', the same as Doxygen's default), and an optional
#   output directory name (the default is 'doxygen-doc').
#
#   Automake Support
#
#   The ../aminclude/doxygen.am file is for use with Automake.
#   Make targets and variables values are controlled by the various
#   DX_COND_* conditionals set by autoconf.
#
#   The provided targets are:
#
#     doxygen-doc: Generate all doxygen documentation.
#
#     doxygen-run: Run doxygen, which will generate some of the
#                  documentation (HTML), but will not do the post processing
#                  required for the rest of it.
#
#   The following variable is intended for use in Makefile.am:
#
#     DX_CLEANFILES = everything to clean.
#
#   Then add this variable to MOSTLYCLEANFILES.
#
# LICENSE
#
#   Copyright (c) 2009 Oren Ben-Kiki <oren@ben-kiki.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
#
# MODIFICATION
#
#   This file has been modified to only provide HTML generation.
#   carstene1ns, 2014
#

#serial 12

## ----------##
## Defaults. ##
## ----------##

DX_ENV=""
AC_DEFUN([DX_FEATURE_doc],  ON)
AC_DEFUN([DX_FEATURE_dot],  OFF)
AC_DEFUN([DX_FEATURE_html], ON)

## --------------- ##
## Private macros. ##
## --------------- ##

# DX_ENV_APPEND(VARIABLE, VALUE)
# ------------------------------
# Append VARIABLE="VALUE" to DX_ENV for invoking doxygen.
AC_DEFUN([DX_ENV_APPEND], [AC_SUBST([DX_ENV], ["$DX_ENV $1='$2'"])])

# DX_DIRNAME_EXPR
# ---------------
# Expand into a shell expression prints the directory part of a path.
AC_DEFUN([DX_DIRNAME_EXPR],
         [[expr ".$1" : '\(\.\)[^/]*$' \| "x$1" : 'x\(.*\)/[^/]*$']])

# DX_IF_FEATURE(FEATURE, IF-ON, IF-OFF)
# -------------------------------------
# Expands according to the M4 (static) status of the feature.
AC_DEFUN([DX_IF_FEATURE], [ifelse(DX_FEATURE_$1, ON, [$2], [$3])])

# DX_REQUIRE_PROG(VARIABLE, PROGRAM)
# ----------------------------------
# Require the specified program to be found for the DX_CURRENT_FEATURE to work.
AC_DEFUN([DX_REQUIRE_PROG], [
AC_PATH_TOOL([$1], [$2])
if test "$DX_FLAG_[]DX_CURRENT_FEATURE$$1" = 1; then
    AC_MSG_WARN([$2 not found - will not DX_CURRENT_DESCRIPTION])
    AC_SUBST(DX_FLAG_[]DX_CURRENT_FEATURE, 0)
fi
])

# DX_TEST_FEATURE(FEATURE)
# ------------------------
# Expand to a shell expression testing whether the feature is active.
AC_DEFUN([DX_TEST_FEATURE], [test "$DX_FLAG_$1" = 1])

# DX_CHECK_DEPEND(REQUIRED_FEATURE, REQUIRED_STATE)
# -------------------------------------------------
# Verify that a required features has the right state before trying to turn on
# the DX_CURRENT_FEATURE.
AC_DEFUN([DX_CHECK_DEPEND], [
test "$DX_FLAG_$1" = "$2" \
|| AC_MSG_ERROR([doxygen-DX_CURRENT_FEATURE ifelse([$2], 1,
                            requires, contradicts) doxygen-DX_CURRENT_FEATURE])
])

# DX_CLEAR_DEPEND(FEATURE, REQUIRED_FEATURE, REQUIRED_STATE)
# ----------------------------------------------------------
# Turn off the DX_CURRENT_FEATURE if the required feature is off.
AC_DEFUN([DX_CLEAR_DEPEND], [
test "$DX_FLAG_$1" = "$2" || AC_SUBST(DX_FLAG_[]DX_CURRENT_FEATURE, 0)
])

# DX_FEATURE_ARG(FEATURE, DESCRIPTION,
#                CHECK_DEPEND, CLEAR_DEPEND,
#                REQUIRE, DO-IF-ON, DO-IF-OFF)
# --------------------------------------------
# Parse the command-line option controlling a feature. CHECK_DEPEND is called
# if the user explicitly turns the feature on (and invokes DX_CHECK_DEPEND),
# otherwise CLEAR_DEPEND is called to turn off the default state if a required
# feature is disabled (using DX_CLEAR_DEPEND). REQUIRE performs additional
# requirement tests (DX_REQUIRE_PROG). Finally, an automake flag is set and
# DO-IF-ON or DO-IF-OFF are called according to the final state of the feature.
AC_DEFUN([DX_ARG_ABLE], [
    AC_DEFUN([DX_CURRENT_FEATURE], [$1])
    AC_DEFUN([DX_CURRENT_DESCRIPTION], [$2])
    AC_ARG_ENABLE(doxygen-$1,
                  [AS_HELP_STRING(DX_IF_FEATURE([$1], [--disable-doxygen-$1],
                                                      [--enable-doxygen-$1]),
                                  DX_IF_FEATURE([$1], [don't $2], [$2]))],
                  [
case "$enableval" in
#(
y|Y|yes|Yes|YES)
    AC_SUBST([DX_FLAG_$1], 1)
    $3
;; #(
n|N|no|No|NO)
    AC_SUBST([DX_FLAG_$1], 0)
;; #(
*)
    AC_MSG_ERROR([invalid value '$enableval' given to doxygen-$1])
;;
esac
], [
AC_SUBST([DX_FLAG_$1], [DX_IF_FEATURE([$1], 1, 0)])
$4
])
if DX_TEST_FEATURE([$1]); then
    $5
    :
fi
AM_CONDITIONAL(DX_COND_$1, DX_TEST_FEATURE([$1]))
if DX_TEST_FEATURE([$1]); then
    $6
    :
else
    $7
    :
fi
])

## -------------- ##
## Public macros. ##
## -------------- ##

# DX_XXX_FEATURE(DEFAULT_STATE)
# -----------------------------
AC_DEFUN([DX_DOXYGEN_FEATURE], [AC_DEFUN([DX_FEATURE_doc],  [$1])])
AC_DEFUN([DX_DOT_FEATURE],     [AC_DEFUN([DX_FEATURE_dot], [$1])])
AC_DEFUN([DX_HTML_FEATURE],    [AC_DEFUN([DX_FEATURE_html], [$1])])

# DX_INIT_DOXYGEN(PROJECT, [CONFIG-FILE], [OUTPUT-DOC-DIR])
# ---------------------------------------------------------
# PROJECT also serves as the base name for the documentation files.
# The default CONFIG-FILE is "Doxyfile" and OUTPUT-DOC-DIR is "doxygen-doc".
AC_DEFUN([DX_INIT_DOXYGEN], [

# Files:
AC_SUBST([DX_PROJECT], [$1])
AC_SUBST([DX_CONFIG], [ifelse([$2], [], Doxyfile, [$2])])
AC_SUBST([DX_DOCDIR], [ifelse([$3], [], doxygen-doc, [$3])])

# Environment variables used inside doxygen.cfg:
DX_ENV_APPEND(SRCDIR, $srcdir)
DX_ENV_APPEND(PROJECT, $DX_PROJECT)
DX_ENV_APPEND(DOCDIR, $DX_DOCDIR)
DX_ENV_APPEND(VERSION, $PACKAGE_VERSION)

# Doxygen itself:
DX_ARG_ABLE(doc, [generate any doxygen documentation],
            [],
            [],
            [DX_REQUIRE_PROG([DX_DOXYGEN], doxygen)
             DX_REQUIRE_PROG([DX_PERL], perl)],
            [DX_ENV_APPEND(PERL_PATH, $DX_PERL)])

# Dot for graphics:
DX_ARG_ABLE(dot, [generate graphics for doxygen documentation],
            [DX_CHECK_DEPEND(doc, 1)],
            [DX_CLEAR_DEPEND(doc, 1)],
            [DX_REQUIRE_PROG([DX_DOT], dot)],
            [DX_ENV_APPEND(HAVE_DOT, YES)
             DX_ENV_APPEND(DOT_PATH, [`DX_DIRNAME_EXPR($DX_DOT)`])],
            [DX_ENV_APPEND(HAVE_DOT, NO)])

# Plain HTML pages generation:
DX_ARG_ABLE(html, [generate doxygen plain HTML documentation],
            [DX_CHECK_DEPEND(doc, 1)],
            [DX_CLEAR_DEPEND(doc, 1)],
            [],
            [DX_ENV_APPEND(GENERATE_HTML, YES)],
            [DX_ENV_APPEND(GENERATE_HTML, NO)])

#For debugging:
#echo DX_FLAG_doc=$DX_FLAG_doc
#echo DX_FLAG_dot=$DX_FLAG_dot
#echo DX_FLAG_html=$DX_FLAG_html
#echo DX_ENV=$DX_ENV
])
