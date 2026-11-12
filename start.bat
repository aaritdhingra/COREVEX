@echo off
REM ============================================================================
REM COREVEX — start.bat (sir ke liye: sirf is file par double-click karo)
REM ----------------------------------------------------------------------------
REM Ye file 3 kaam karti hai, ek ke baad ek:
REM   1. build_core.bat se app build karti hai (BUILD_OK aana chahiye)
REM   2. windeployqt se Qt ki zaroori files copy karti hai
REM   3. build\COREVEX.exe chala deti hai
REM Pehli baar build me kuch minute lag sakte hain. Agar build fail ho,
REM to ye window khuli reh jayegi aur error dikhega — use copy karke bhej do.
REM Requirements (ek baar install karne hain): Windows 10/11 x64,
REM Visual Studio 2022 (C++), Qt 6.7.3 (C:\Qt\6.7.3\msvc2019_64),
REM CMake aur Ninja.
REM ============================================================================
cd /d %~dp0
echo COREVEX build ho raha hai, thoda wait karo...
call build_core.bat
if errorlevel 1 (
  echo.
  echo BUILD FAIL ho gaya. Upar wala error dekho.
  pause
  exit /b 1
)
echo.
echo Qt files copy ho rahi hain...
"C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe" build\COREVEX.exe
if errorlevel 1 (
  echo.
  echo windeployqt FAIL ho gaya. Upar wala error dekho.
  pause
  exit /b 1
)
echo.
echo COREVEX khul raha hai...
start "" "build\COREVEX.exe"
