@echo off

set PATH=%CD%;%PATH%
set /p unique_id="Insert cia Unique ID [0-9, A-F] (Example: AAAAAA): "
set /p title="Insert Game Title: "
set /p author="Insert Game Author: "
cd app
echo Creating icon and banner files...
mkdir ..\tmp
bannertool makebanner -i ../assets/banner.png -a ../assets/audio.wav -o ../tmp/banner.bin
bannertool makesmdh -s "%title%" -l "%title%" -p "%author%" -i ../assets/icon.png -o ../tmp/icon.bin
echo Creating romfs file...
3dstool -cvtf romfs ../tmp/romfs.bin --romfs-dir ../romfs
echo Building cia file...
powershell -Command "(gc cia_workaround.rsf) -replace '(UniqueId\s+:)\s*.*$', '${1} 0x%unique_id%' | Out-File cia_workaround.rsf"
makerom -f cia -o ../my_game.cia -elf easyrpg-player.elf -rsf cia_workaround.rsf -icon ../tmp/icon.bin -banner ../tmp/banner.bin -exefslogo -target t -romfs ../tmp/romfs.bin
cd ..
echo Deleting temp files...
rmdir /s /q tmp
echo Finished! Enjoy EasyRPG Player!
set /p dummy="Press ENTER to exit"
