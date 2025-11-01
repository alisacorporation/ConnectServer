@echo off
REM Build script for ConnectServer (Windows)

setlocal enabledelayedexpansion

echo === ConnectServer Build Script ===
echo.

REM Parse arguments
set BUILD_TYPE=Release
set CLEAN_BUILD=0
set USE_NCURSES=OFF
set BUILD_TESTS=OFF

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if /i "%~1"=="--tests" (
    set BUILD_TESTS=ON
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    echo Usage: %0 [options]
    echo Options:
    echo   --debug     Build in Debug mode (default: Release)
    echo   --clean     Clean build directory before building
    echo   --tests     Build unit tests
    echo   --help      Show this help message
    exit /b 0
)
echo Unknown option: %~1
exit /b 1

:end_parse

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: cmake is required but not found in PATH
    exit /b 1
)

REM Create build directory
set BUILD_DIR=build
if %CLEAN_BUILD%==1 (
    if exist "%BUILD_DIR%" (
        echo Cleaning build directory...
        rmdir /s /q "%BUILD_DIR%"
    )
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Configure
echo Configuring...
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if "%BUILD_TESTS%"=="ON" (
    set CMAKE_ARGS=!CMAKE_ARGS! -DBUILD_TESTS=ON
)

REM Auto-detect vcpkg toolchain
if defined VCPKG_ROOT (
    set CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
    echo Using vcpkg from: %VCPKG_ROOT%
) else if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    echo Using vcpkg from: C:\vcpkg
) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%/vcpkg/scripts/buildsystems/vcpkg.cmake
    echo Using vcpkg from: %USERPROFILE%\vcpkg
)

cmake .. %CMAKE_ARGS%
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

REM Copy config files from config folder to build directory
echo Copying configuration files from config folder...
if exist "..\config\ConnectServer.ini.example" (
    copy /Y "..\config\ConnectServer.ini.example" "%BUILD_TYPE%\ConnectServer.ini" >nul
    echo   [OK] ConnectServer.ini (from config/ConnectServer.ini.example)
)
if exist "..\config\ServerList.dat.example" (
    copy /Y "..\config\ServerList.dat.example" "%BUILD_TYPE%\ServerList.dat" >nul
    echo   [OK] ServerList.dat (from config/ServerList.dat.example)
)

echo.
echo Build completed successfully!
echo Binary location: %BUILD_DIR%\%BUILD_TYPE%\ConnectServer.exe
echo.
echo To run the server:
echo   cd %BUILD_DIR%\%BUILD_TYPE%
echo   ConnectServer.exe

endlocal
