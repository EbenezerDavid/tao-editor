# install_windows.ps1
Write-Host "Starting installation for Windows (MSYS2)..."

# 下载MSYS2
Invoke-WebRequest -Uri "https://github.com/msys2/msys2-installer/releases/download/2023-05-26/msys2-x86_64-20230526.exe" -OutFile "msys2-installer.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to download MSYS2. Please check your internet connection."
    Pause
    exit 1
}

# 安装MSYS2
Start-Process -Wait -FilePath "msys2-installer.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "MSYS2 installation failed."
    Pause
    exit 1
}

# 清理安装器
Remove-Item "msys2-installer.exe"

# 打开MSYS2安装依赖
Start-Process -Wait -FilePath "C:\msys64\msys2.exe" -ArgumentList "-c", "pacman -Syu && pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config --noconfirm"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to start MSYS2 or install dependencies."
    Pause
    exit 1
}

Write-Host "Installation complete! Please open 'MSYS2 MinGW 64-bit' terminal and run:"
Write-Host "gcc -o aee tao-editor.c $(pkg-config --cflags --libs gtk+-3.0)"
Write-Host "Then run: ./aee.exe test.txt"
Pause