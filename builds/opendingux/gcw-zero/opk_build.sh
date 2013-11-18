#!/bin/sh
rm -rf EasyRPG.opk
mksquashfs EasyRPG easyrpg.png run.sh default.gcw0.desktop EasyRPG.opk -all-root -noappend -no-exports -no-xattrs
