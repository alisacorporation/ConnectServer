@echo off
REM Cleanup script to remove duplicate config files from root directory
REM These files are now managed in the config/ folder

echo === ConnectServer Config Cleanup Script ===
echo.
echo This script will remove duplicate configuration files from the root directory.
echo Configuration files are now managed in the config/ folder.
echo.

set ROOT_DIR=%~dp0..
cd /d "%ROOT_DIR%"

echo Current directory: %CD%
echo.

REM Check if files exist
set FILES_TO_REMOVE=0

if exist "ConnectServer.ini.example" (
    echo [FOUND] ConnectServer.ini.example
    set /a FILES_TO_REMOVE+=1
)

if exist "ServerList.dat.example" (
    echo [FOUND] ServerList.dat.example
    set /a FILES_TO_REMOVE+=1
)

if %FILES_TO_REMOVE%==0 (
    echo No duplicate files found. Root directory is already clean.
    echo.
    pause
    exit /b 0
)

echo.
echo Files to remove: %FILES_TO_REMOVE%
echo.
echo WARNING: This will delete the above files from the root directory.
echo The same files exist in config/ folder and will be used by the build system.
echo.
set /p CONFIRM="Do you want to continue? (Y/N): "

if /i not "%CONFIRM%"=="Y" (
    echo Operation cancelled.
    pause
    exit /b 0
)

echo.
echo Removing files...

if exist "ConnectServer.ini.example" (
    del "ConnectServer.ini.example"
    if %ERRORLEVEL%==0 (
        echo   [OK] Removed ConnectServer.ini.example
    ) else (
        echo   [ERROR] Failed to remove ConnectServer.ini.example
    )
)

if exist "ServerList.dat.example" (
    del "ServerList.dat.example"
    if %ERRORLEVEL%==0 (
        echo   [OK] Removed ServerList.dat.example
    ) else (
        echo   [ERROR] Failed to remove ServerList.dat.example
    )
)

echo.
echo Cleanup completed!
echo.
echo Configuration files are now managed in config/ folder:
echo   - config/ConnectServer.ini.example
echo   - config/ServerList.dat.example
echo.
echo The build system will automatically copy these to the output directory.
echo.
pause
