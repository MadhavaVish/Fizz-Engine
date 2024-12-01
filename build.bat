@echo off
@REM REM Set the vcpkg root directory (adjust this path to your vcpkg installation)
@REM set VCPKG_ROOT=C:\vcpkg

REM Set the SDL2_DIR or CMAKE_PREFIX_PATH if SDL2 is installed in a custom location
REM Uncomment one of the following lines if needed

@REM set SDL2_DIR=C:\vcpkg\installed\x64-windows\share\sdl2
REM set CMAKE_PREFIX_PATH=C:\vcpkg\installed\x64-windows\share\sdl2

REM Create the build directory if it doesn't exist
if not exist build (
    mkdir build
)

REM Navigate to the build directory
cd build

REM Run CMake configuration with vcpkg toolchain file
cmake ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DCMAKE_BUILD_TYPE=Release ^
    ..

REM Build the project
cmake --build . --config Release

REM Navigate back to the project root
cd ..

@echo Build process completed.