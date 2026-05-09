@echo off
title Library System - Windows Build

echo.
echo  ============================================================
echo   LIBRARY SYSTEM - Windows Build Script
echo  ============================================================
echo.

:: Check CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo  [ERROR] CMake is NOT installed!
    echo  Download from: https://cmake.org/download/
    echo  During install tick: Add CMake to system PATH
    pause
    exit /b 1
)
echo  [OK] CMake found

:: Check g++
g++ --version >nul 2>&1
if errorlevel 1 (
    echo  [ERROR] MinGW g++ is NOT installed!
    echo  Download from: https://github.com/niXman/mingw-builds-binaries/releases
    echo  Extract to C:\ and add C:\mingw64\bin to PATH
    pause
    exit /b 1
)
echo  [OK] MinGW g++ found

:: Create data folder
if not exist "data" mkdir data

:: Delete old build folder if exists
if exist "build" rmdir /s /q build
mkdir build
cd build

echo.
echo  [1/2] Configuring CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo  [ERROR] CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo  [2/2] Compiling...
cmake --build . --config Release
if errorlevel 1 (
    echo  [ERROR] Compilation failed!
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo  ============================================================
echo   BUILD SUCCESSFUL!
echo   Double-click run.bat to start the program
echo  ============================================================
echo.

:: Create run.bat
echo @echo off > run.bat
echo cd /d "%~dp0" >> run.bat
echo build\library.exe >> run.bat
echo pause >> run.bat

echo  [OK] run.bat created!
echo.
pause
