#!/bin/bash

# 检测操作系统
OS=$(uname -s)
echo "Detected OS: $OS"

# 检查命令是否存在
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 安装依赖函数
install_deps() {
    if [ "$OS" = "Linux" ]; then
        # 检查发行版
        if command_exists apt; then
            # Ubuntu/Debian
            echo "Installing dependencies for Ubuntu/Debian..."
            sudo apt update
            sudo apt install -y build-essential libgtk-3-dev pkg-config
            sudo apt install -y build-essential libgtk-3-dev libgtksourceview-3.0-dev gcc
        elif command_exists yum; then
            # CentOS/RHEL
            echo "Installing dependencies for CentOS/RHEL..."
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y gtk3-devel pkgconf
        else
            echo "Unsupported Linux distribution. Please install GCC, GTK+ 3, and pkg-config manually."
            exit 1
        fi
    elif [ "$OS" = "Darwin" ]; then
        # macOS
        echo "Installing dependencies for macOS..."
        if ! command_exists brew; then
            echo "Installing Homebrew..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        fi
        brew install gcc gtk+3 pkg-config
        # 根据架构设置PKG_CONFIG_PATH
        if [ $(uname -m) = "arm64" ]; then
            echo 'export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.zshrc
        else
            echo 'export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.zshrc
        fi
        source ~/.zshrc
    else
        echo "Unsupported OS: $OS. For Windows, please install MSYS2 and follow manual instructions."
        exit 1
    fi
}

# 验证安装
verify_install() {
    echo "Verifying installation..."
    if ! command_exists gcc; then
        echo "GCC not found. Installation failed."
        exit 1
    fi
    if ! command_exists pkg-config; then
        echo "pkg-config not found. Installation failed."
        exit 1
    fi
    if ! pkg-config --modversion gtk+-3.0 >/dev/null 2>&1; then
        echo "GTK+ 3 not found. Installation failed."
        exit 1
    fi
    echo "All dependencies installed successfully!"
    echo "GCC version: $(gcc --version | head -n 1)"
    echo "GTK+ version: $(pkg-config --modversion gtk+-3.0)"
}

# 主流程
echo "Starting installation of dependencies for AI Easy Edition..."

install_deps
verify_install

echo "Dependencies installed. You can now compile the program with:"
echo "gcc -o tao-editor tao-editor.c \$(pkg-config --cflags --libs gtk+-3.0)"
echo "Then run with: ./aee test.txt (Linux/macOS) or ./aee.exe test.txt (Windows)"