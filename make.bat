@ECHO OFF

if not exist "gbdk\" (
    echo Couldn't find gbdk directory in current directory
    exit /b
)

%cd%/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o dist/main.o src/main.c
%cd%/gbdk/bin/lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o dist/main.gb dist/main.o