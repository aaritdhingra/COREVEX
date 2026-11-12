@echo off
REM ============================================================================
REM SECTION — BUILD SCRIPT (laptop, Qt 6.7.3, koi extra install nahi)
REM ----------------------------------------------------------------------------
REM Qt laptop par pehle se hai: C:\Qt\6.7.3\msvc2019_64 (aqt log se verified).
REM Ye script sirf us Qt ko use karta hai, naya Qt download nahi karta.
REM Chalane ka tareeka: Developer PowerShell / Developer Command Prompt se
REM   build_core.bat
REM Agar Ninja na mile to: winget install Ninja-build.Ninja
REM ============================================================================
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set CMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
set PATH=C:\Qt\6.7.3\msvc2019_64\bin;%PATH%
cd /d %~dp0
REM Purani build folder (dusri machine/path ki CMakeCache) ko hatao, warna CMake stale cache error dega.
if exist build rmdir /s /q build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
if errorlevel 1 exit /b 1
cmake --build build
if errorlevel 1 exit /b 1
echo BUILD_OK
echo Run: build\COREVEX.exe  (pehle windeployqt chahiye agar folder bahar le ja rahe ho)
