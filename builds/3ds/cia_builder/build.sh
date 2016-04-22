#!/bin/bash

# Abort on error
set -e

echo -n "Insert cia Unique ID (Example: 0xAAAAAA): "
read unique_id
echo -n "Insert Game Title: "
read title
echo -n "Insert Game Author: "
read author
cd app
echo "Creating icon and banner files..."
bannertool makebanner -i ../assets/banner.png -a ../assets/audio.wav -o ../tmp/banner.bin
bannertool makesmdh -s "$title" -l "$title" -p "$author" -i ../assets/icon.png -o ../tmp/icon.bin
echo "Creating romfs file..."
mkdir -p ../tmp
3dstool -cvtf romfs ../tmp/romfs.bin --romfs-dir ../romfs
echo "Building cia file..."
hex_set $unique_id
makerom -f cia -o ../my_game.cia -elf easyrpg-player.elf -rsf cia_workaround.rsf -icon ../tmp/icon.bin -banner ../tmp/banner.bin -exefslogo -target t -romfs ../tmp/romfs.bin
cd ..
echo "Deleting temp files..."
rm -rf tmp
echo "Finished! Enjoy EasyRPG Player!"
