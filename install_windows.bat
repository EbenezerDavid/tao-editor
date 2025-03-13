@echo off
echo Starting installation for Windows (MSYS2)...

REM 检查是否已安装MSYS2
if not exist "C:\msys64" (
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

    del msys2-installer.exe
) else (
    echo MSYS2 already installed at C:\msys64.
)

REM 安装依赖
echo Installing dependencies...
start /wait "C:\msys64\msys2.exe" -c "pacman -Syu --noconfirm && pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config --noconfirm"
if %ERRORLEVEL% neq 0 (
    echo Failed to install dependencies. Please open 'MSYS2 MSYS' terminal and run:
    echo pacman -Syu
    echo pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config --noconfirm
    echo pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3
    pause
    exit /b 1
)

REM 编译程序
echo Compiling tao-editor.c...
start /wait "C:\msys64\mingw64.exe" -c "cd /c/Users/tao/DOWNLOADS/tao-editor && gcc -o aee tao-editor.c $(pkg-config --cflags --libs gtk+-3.0)"
if %ERRORLEVEL% neq 0 (
    echo Compilation failed. Please ensure tao-editor.c exists in C:\Users\tao\DOWNLOADS\tao-editor.
    pause
    exit /b 1
)

REM 使用ldd查找依赖并复制DLL
echo Copying required DLLs...
set "DLL_LIST=C:\Users\tao\DLL_LIST.txt"
start /wait "C:\msys64\mingw64.exe" -c "cd /c/Users/tao/DOWNLOADS/tao-editor && ldd aee.exe | grep mingw64 | awk '{print $3}' > /c/Users/tao/DLL_LIST.txt"
for /f "tokens=*" %%i in (%DLL_LIST%) do (
    copy "%%i" "C:\Users\tao\DOWNLOADS\tao-editor" /Y
)
del %DLL_LIST%

echo Installation and setup complete!
echo You can now run the program directly:
echo cd C:\Users\tao\DOWNLOADS\tao-editor
echo aee.exe test.txt
echo Press any key to exit...
pause