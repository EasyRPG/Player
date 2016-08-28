@echo off

set PATH=%CD%;%PATH%
set /p id="Insert vpk title ID [4 characters, only uppercase] (Example: AAAA): "
set /p title="Insert Game Title: "
vita-mksfoex -s TITLE_ID=%id%00001 "%title%" build\sce_sys\param.sfo
pngquant assets\bg.png -o build\sce_sys\livearea\contents\bg.png
pngquant assets\icon0.png -o build\sce_sys\icon0.png
pngquant assets\startup.png -o build\sce_sys\livearea\contents\startup.png
echo %id%00001> build\titleid.txt
7z a -tzip "%title%.vpk" -r .\build\* .\build\eboot.bin