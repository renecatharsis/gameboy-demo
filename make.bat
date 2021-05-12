@ECHO OFF

if not exist "lib\gbdk\" (
    echo Couldn't find gbdk
    exit /b
)

if not exist "lib\mod2gbt\mod2gbt.exe" (
    echo Couldn't find mod2gbt
    exit /b
)

%cd%/lib/mod2gbt/mod2gbt.exe res/music.mod song -c 2
move output.c dist/output.c >nul

%cd%/lib/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o dist/main.o src/main.c
%cd%/lib/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o dist/output.o dist/output.c
%cd%/lib/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o dist/gbt_player.o src/gbtplayer/gbt_player.s
%cd%/lib/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o dist/gbt_player_bank1.o src/gbtplayer/gbt_player_bank1.s
%cd%/lib/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -Wl-yt1 -Wl-yo4 -Wl-ya0 -o dist/main.gb dist/main.o dist/output.o dist/gbt_player.o dist/gbt_player_bank1.o

cd dist
del *.c *.o *.lst
cd ..