@echo off
REM Windows build script for VCS
REM Requires: CMake, Visual Studio Build Tools, OpenSSL, nlohmann_json

REM Check for CMake
where cmake >nul 2>nul || (
    echo CMake not found. Please install CMake and add it to your PATH.
    exit /b 1
)

REM Check for cl (MSVC compiler)
where cl >nul 2>nul || (
    echo MSVC compiler not found. Please install Visual Studio Build Tools and add to PATH.
    exit /b 1
)

REM Check for vcpkg
where vcpkg >nul 2>nul || (
    echo vcpkg not found. Please install vcpkg and add it to your PATH.
    exit /b 1
)

REM Install dependencies with vcpkg
vcpkg install openssl nlohmann-json

REM Configure and build
if not exist build mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ..
nmake
copy vcs.exe ..
echo Build complete! You can now use vcs.exe from this directory.
