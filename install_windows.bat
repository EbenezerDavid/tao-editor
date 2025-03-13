@echo off
echo Starting installation for Windows (MSYS2)...

REM 下载并安装MSYS2
echo Downloading MSYS2...
powershell -Command "Invoke-WebRequest -Uri 'https://github.com/msys2/msys2-installer/releases/download/2023-05-26/msys2-x86_64-20230526.exe' -OutFile 'msys2-installer.exe'"
if %ERRORLEVEL% neq 0 (
    echo Failed to download MSYS2. Please check your internet connection.
    pause
    exit /b 1
)

echo Installing MSYS2...
start /wait msys2-installer.exe
if %ERRORLEVEL% neq 0 (
    echo MSYS2 installation failed.
    pause
    exit /b 1
)

REM 清理安装器
del msys2-installer.exe

REM 打开MSYS2终端并安装依赖
echo Opening MSYS2 to install dependencies...
start "" "C:\msys64\msys2.exe" -c "pacman -Syu && pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config --noconfirm"
if %ERRORLEVEL% neq 0 (
    echo Failed to start MSYS2 or install dependencies.
    pause
    exit /b 1
)

echo Installation complete! Please open 'MSYS2 MinGW 64-bit' terminal and run:
echo gcc -o aee ai-easy-edition.c $(pkg-config --cflags --libs gtk+-3.0)
echo Then run: ./aee.exe test.txt
echo Press any key to exit...
pause