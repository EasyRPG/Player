#!/bin/sh

# Based on GIT-VERSION-GEN from git source code

set -e
#set -x

nl='
'

# check if there is a reachable tag
if test -d ${GIT_DIR:-.git} -o -f .git &&
  version=`git describe --tags --long --match "[0-9].[0-9]*" HEAD 2>/dev/null` &&
  case "$version" in
    *$nl*) (exit 0) ;;
    [0-9].[0-9]*)
      # check if the working tree is dirty
      git update-index -q --refresh
      test -z "`git diff-index --name-only HEAD --`" || version="$version-dirty" ;;
  esac
then
  # split for pretty output
  tag=`echo "$version" | cut -s -d- -f1`
  commits=`echo "$version" | cut -s -d- -f2`
  hash=`echo "$version" | cut -s -d- -f3 | cut -c2-`
  dirty=`echo "$version" | cut -s -d- -f4`

  # remove tag, substitute delimiters, format for output
  version=`echo "$version" | sed 's/\([^-]*\)-\([^-]*\)-g\([^-]*\)/git+\2@\3/'`;
else
  # git not available or version not found
  exit 0
fi

if test "$commits" -eq 0
then
  # building a tag
  exit 0
fi

# output
if test "$1" = "--pretty"
then
  echo "Building from git repository:"
  echo "  -$commits commits since tag \"$tag\", object hash is $hash."
  if test -n "$dirty"
  then
    echo "  -You have uncommitted changes."
  fi
else
  echo "$version"
fi
