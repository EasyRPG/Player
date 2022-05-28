#!/bin/bash
# Packages EasyRPG-Player as a macOS .app bundle

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASE_DIR=$SCRIPT_DIR/../..

BUNDLE_NAME="Player.app"
EASYRPG_BINARY="easyrpg-player"

mkdir -p "$BUNDLE_NAME/Contents/MacOS"
mkdir -p "$BUNDLE_NAME/Contents/Resources"
cp "$BASE_DIR/resources/macos/Info.plist" "$BUNDLE_NAME/Contents"
cp "$BASE_DIR/resources/macos/Player.icns" "$BUNDLE_NAME/Contents/Resources"
cp "$EASYRPG_BINARY" "$BUNDLE_NAME/Contents/MacOS/EasyRPG Player"

# TODO: codesign

# TODO: app notarization
