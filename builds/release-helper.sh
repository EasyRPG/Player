#!/bin/bash

# release-helper.sh - maintainer utility script to change the release version
# by carstene1ns 2021, released under the MIT license

set -e

version=$1

if [[ -z $version ]]; then

  echo "No new version argument, aborting!"
  exit 1

fi

if [[ ! $version =~ ^[0-9]\.[0-9]\.[0-9]$ ]]; then

  echo "Invalid version argument. Only digits and dots allowed."
  exit 1

fi

echo "Updating Version in:"

echo "  CMakeLists.txt"
sed -i "/EasyRPG_Player VERSION/,1 s/[0-9]\.[0-9]\.[0-9]/$version/" CMakeLists.txt

echo "  configure.ac"
sed -i "/AC_INIT/,1 s/[0-9]\.[0-9]\.[0-9]/$version/" configure.ac

echo "  builds/android/gradle.properties"
_android_commits=`git rev-list HEAD --count`
sed -i -e "/VERSION_NAME/,1 s/[0-9]\.[0-9]\.[0-9]/$version/" \
       -e "/VERSION_CODE/,1 s/[0-9]\+/${_android_commits}/" builds/android/gradle.properties

echo "  resources/wii/meta.xml"
_wiidate=$(date +%Y%m%d000000)
sed -i -e "/version/,1 s/[0-9]\.[0-9]\.[0-9]/$version/" \
       -e "/release_date/,1 s/[0-9]\{14\}/$_wiidate/" resourses/wii/meta.xml

echo "  resources/psvita/template.xml"
sed -i "/EasyRPG Player/,1 s/[0-9]\.[0-9]\.[0-9]/$version/" resources/psvita/template.xml

echo "  resources/osx/Info.plist"
sed -i "/CFBundleShortVersionString/,+1 s/[0-9]\.[0-9]\.[0-9]/$version/" \
  resources/osx/Info.plist

echo "  resources/player.rc"
sed -i "s/[0-9]\.[0-9]\.[0-9]/$version/; s/[0-9],[0-9],[0-9]/${version//./,}/" \
  resources/player.rc

echo "  src/version.h"
_maj=${version%%.*}
_pat=${version##*.}
_min=${version%.*}
_min=${_min#*.}
sed -i -e "s/\(#define PLAYER_MAJOR\).*/\1 $_maj/" \
       -e "s/\(#define PLAYER_MINOR\).*/\1 $_min/" \
       -e "s/\(#define PLAYER_PATCH\).*/\1 $_pat/" src/version.h

echo "  README.md"
sed -i "s/\(easyrpg-player-\)[0-9]\.[0-9]\.[0-9]/\1$version/g" README.md

cat << EOF

If everything is ready and committed, use these commands to publish the git tag:
$ git tag -a (-s) $version -m "Codename \"\fancy codename\""
$ git push (-n) --tags upstream
EOF
