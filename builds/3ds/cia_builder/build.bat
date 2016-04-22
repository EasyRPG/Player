@echo off

set /p unique_id="Insert cia Unique ID (Example: 0xAAAAAA): "
set /p title="Insert Game Title: "
set /p author="Insert Game Author: "
cd app
echo Creating icon and banner files...
bannertool makebanner -i ../files/banner.png -a ../files/audio.wav -o ../tmp/banner.bin
bannertool makesmdh -s "%title%" -l "%title%" -p "%author%" -i ../files/icon.png -o ../tmp/icon.bin
echo Creating romfs file...
mkdir ..\tmp
3dstool -cvtf romfs ../tmp/romfs.bin --romfs-dir ../romfs
echo Building cia file...
hex_set %unique_id%
makerom -f cia -o ../my_game.cia -elf easyrpg-player.elf -rsf cia_workaround.rsf -icon ../tmp/icon.bin -banner ../tmp/banner.bin -exefslogo -target t -romfs ../tmp/romfs.bin
cd ..
echo Deleting temp files...
rmdir /s /q tmp
echo Finished! Enjoy EasyRPG Player!
set /p dummy="Press ENTER to exit"
