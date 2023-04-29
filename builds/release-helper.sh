#!/bin/bash

# release-helper.sh - maintainer utility script to change the release version
# by carstene1ns 2021, released under the MIT license

set -e

version=$1
verbose=${2:-quiet}

if [[ -z $version ]]; then

  echo "No new version argument, aborting!"
  exit 1

fi

if [[ ! $version =~ ^[0-9](\.[0-9]){1,3}$ ]]; then

  echo "Invalid version argument. Only digits and dots allowed."
  echo "Example: 0.8 or 0.7.0.1"
  exit 1

fi

# helpers
reset="\e[0m"
red="\e[31m"
yellow="\e[33m"
blue="\e[36m"
bold="\e[1m"
function print_file() {
  if [ $verbose == "verbose" ]; then
    echo -e "$yellow  $file:$reset"
  else
    echo -e "$yellow  $file$reset"
  fi
}

function print_verbose() {
  if [ $verbose == "verbose" ]; then
    echo -ne "$blue"
    grep "$1" $2
    echo -e "$reset"
  fi
}

# convenience variables
IFS='.' read -r _maj _min _pat _twk <<< $version
# default to zero
_pat=${_pat:-0}
_twk=${_twk:-0}

if [ $_pat == 0 ]; then
  lcfversion="${_maj}.${_min}"
else
  lcfversion="${_maj}.${_min}.${_pat}"
fi

echo "Updating Version in:"

file="CMakeLists.txt"
print_file
sed -i "/EasyRPG_Player VERSION/,1 s/[0-9]\(.[0-9]\)\{1,3\}/$version/" $file
sed -i "/liblcf VERSION/,1 s/[0-9]\(.[0-9]\)\{1,3\}/$lcfversion/" $file
print_verbose " VERSION " $file

file=configure.ac
print_file
sed -i -e "/ep_version_major/,1 s/\[[0-9]\+\]/[$_maj]/" \
       -e "/ep_version_minor/,1 s/\[[0-9]\+\]/[$_min]/" \
       -e "/ep_version_patch/,1 s/\[[0-9]\+\]/[$_pat]/" \
       -e "/ep_version_tweak/,1 s/\[[0-9]\+\]/[$_twk]/" $file
sed -i "/liblcf >= /,1 s/[0-9]\(.[0-9]\)\{1,3\}/$lcfversion/" $file
print_verbose 'm4_define(\[ep_version_' $file
print_verbose "liblcf >= [0-9]" $file

# + 2 because of two extra commits: version commit itself & merge commit
file="builds/android/gradle.properties"
print_file
_android_commits=$((`git rev-list HEAD --count` + 2))
sed -i -e "/VERSION_NAME/,1 s/[0-9]\(.[0-9]\)\{1,3\}/$version/" \
       -e "/VERSION_CODE/,1 s/[0-9]\+/${_android_commits}/" $file
print_verbose 'VERSION_.*=[0-9]' $file

file="src/version.cpp"
print_file
sed -i -e "s/\(#define EP_VERSION_MAJOR\).*/\1 $_maj/" \
       -e "s/\(#define EP_VERSION_MINOR\).*/\1 $_min/" \
       -e "s/\(#define EP_VERSION_PATCH\).*/\1 $_pat/" \
       -e "s/\(#define EP_VERSION_TWEAK\).*/\1 $_twk/" $file
print_verbose "define EP_VERSION_.*[0-9]" $file

file="docs/BUILDING.md"
print_file
sed -i "s/\(easyrpg-player-\)[0-9]\(.[0-9]\)\{1,3\}/\1$version/g" $file
print_verbose "easyrpg-player-[0-9]" $file

echo -e "$(
cat << EOM

${red}Please check ${bold}docs/BUILDING.md${reset}${red} file for whitespace problems.${reset}

If everything is ready and committed, use these commands to publish the git tag:
$ git tag -a (-s) $version -m "Codename \"fancy codename\""
$ git push (-n) --tags upstream
EOM
)"
