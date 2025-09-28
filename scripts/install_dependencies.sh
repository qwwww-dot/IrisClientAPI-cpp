#!/bin/bash

echo "Installing dependencies for IrisCPP..."

# Detect package manager and install dependencies
if command -v apt-get &> /dev/null; then
    # Debian/Ubuntu
    echo "Installing system dependencies..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake git libssl-dev pkg-config curl zip unzip tar
elif command -v dnf &> /dev/null; then
    # Fedora
    echo "Installing system dependencies..."
    sudo dnf groupinstall -y "Development Tools"
    sudo dnf install -y cmake git openssl-devel pkg-config curl zip unzip tar
elif command -v pacman &> /dev/null; then
    # Arch Linux
    echo "Installing system dependencies..."
    sudo pacman -Sy base-devel cmake git openssl pkg-config curl zip unzip tar
elif command -v zypper &> /dev/null; then
    # OpenSUSE
    echo "Installing system dependencies..."
    sudo zypper install -y gcc-c++ cmake git libopenssl-devel pkg-config curl zip unzip tar pattern:devel_basis
elif command -v apk &> /dev/null; then
    # Alpine Linux
    echo "Installing system dependencies..."
    sudo apk add build-base cmake git openssl-dev pkgconfig curl zip unzip tar
    export VCPKG_FORCE_SYSTEM_BINARIES=1
else
    echo "Unsupported Linux distribution. Please install required packages manually:"
    echo "- C++ compiler (g++ or clang++)"
    echo "- CMake"
    echo "- Git"
    echo "- pkg-config"
    echo "- SSL development files"
    echo "- curl, zip, unzip, tar"
    exit 1
fi

# Check if vcpkg is installed
if [ ! -d "$HOME/vcpkg" ]; then
    echo "Installing vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git "$HOME/vcpkg"
    "$HOME/vcpkg/bootstrap-vcpkg.sh"
fi

# Add vcpkg to PATH
export PATH="$PATH:$HOME/vcpkg"

echo "Installing nlohmann/json..."
mkdir -p include/nlohmann
curl -L https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -o include/nlohmann/json.hpp

echo "Installing vcpkg packages..."
"$HOME/vcpkg/vcpkg" install nlohmann-json:x64-linux --debug

# Install for system integration
"$HOME/vcpkg/vcpkg" integrate install

# Create or update vcpkg config for CMake
VCPKG_CMAKE_CONFIG="$HOME/.vcpkg/vcpkg.cmake"
mkdir -p "$(dirname "$VCPKG_CMAKE_CONFIG")"
echo "set(CMAKE_TOOLCHAIN_FILE \"$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake\")" > "$VCPKG_CMAKE_CONFIG"

echo "Dependencies installed successfully!
To build the project, use:
mkdir -p build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"