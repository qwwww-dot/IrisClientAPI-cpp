@echo off
echo Installing dependencies for IrisCPP...

:: Check if vcpkg is installed
if not exist "%USERPROFILE%\vcpkg" (
    echo Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git "%USERPROFILE%\vcpkg"
    "%USERPROFILE%\vcpkg\bootstrap-vcpkg.bat"
)

:: Add vcpkg to PATH
set PATH=%PATH%;%USERPROFILE%\vcpkg

:: Install required packages
vcpkg install curl:x64-windows
vcpkg install nlohmann-json:x64-windows

:: Install for Visual Studio integration
vcpkg integrate install

echo Dependencies installed successfully!
pause