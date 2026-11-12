@echo off
REM ============================================================================
REM COREVEX — setup.bat (sir ke liye: SIRF EK BAAR chalana hai)
REM ----------------------------------------------------------------------------
REM Ye file sab kuch khud kar deti hai, ek ke baad ek:
REM   1. CMake aur Ninja install karti hai (winget se, agar nahi hain)
REM   2. Visual Studio 2022 + C++ tools install karti hai (winget se, agar nahi hain)
REM   3. Qt 6.7.3 install karti hai (aqtinstall se, agar nahi hai)
REM   4. App build karti hai aur Qt ki zaroori files copy karti hai
REM Uske baad app chalane ke liye har baar sirf run.bat par double-click karo.
REM
REM Pehli baar me time lagega (VS + Qt bade downloads hain, 20-45 minute),
REM internet chahiye, aur Admin / Yes wala prompt aaye to Yes dabana hai.
REM Agar koi step fail ho to window khuli reh jayegi — error copy karke bhej do.
REM ============================================================================
cd /d %~dp0
echo ================================================
echo  COREVEX setup shuru ho raha hai...
echo ================================================
echo.

REM --- winget check ---
where winget >nul 2>&1
if errorlevel 1 (
  echo winget nahi mila. Microsoft Store kholo, "App Installer" update karo,
  echo phir dobara setup.bat chalao.
  pause
  exit /b 1
)

REM --- 1. CMake ---
where cmake >nul 2>&1
if errorlevel 1 (
  echo [1/5] CMake install ho raha hai...
  winget install -e --id Kitware.CMake --accept-source-agreements --accept-package-agreements --silent
  set "PATH=C:\Program Files\CMake\bin;%PATH%"
)
where cmake >nul 2>&1
if errorlevel 1 (
  echo CMake abhi bhi nahi mila. Dobara setup.bat chalao.
  pause
  exit /b 1
)
echo [1/5] CMake mil gaya.

REM --- 2. Ninja ---
where ninja >nul 2>&1
if errorlevel 1 (
  echo [2/5] Ninja install ho raha hai...
  winget install -e --id Ninja-build.Ninja --accept-source-agreements --accept-package-agreements --silent
  for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_*") do set "PATH=%%D;%PATH%"
)
where ninja >nul 2>&1
if errorlevel 1 (
  echo Ninja abhi bhi nahi mila. Dobara setup.bat chalao.
  pause
  exit /b 1
)
echo [2/5] Ninja mil gaya.

REM --- 3. Visual Studio 2022 + C++ tools ---
if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
  echo [3/5] Visual Studio 2022 install ho raha hai, isme sabse zyada time lagega...
  winget install -e --id Microsoft.VisualStudio.2022.Community --accept-source-agreements --accept-package-agreements --silent --override "--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --quiet --norestart"
)
if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
  echo Visual Studio install nahi hua. Dobara setup.bat chalao.
  pause
  exit /b 1
)
echo [3/5] Visual Studio mil gaya.

REM --- 4. Qt 6.7.3 (WebEngine ke saath) ---
if not exist "C:\Qt\6.7.3\msvc2019_64\bin\qmake.exe" (
  echo [4/5] Qt 6.7.3 install ho raha hai, bada download hai, time lagega...
  where python >nul 2>&1
  if errorlevel 1 (
    echo Python install ho raha hai...
    winget install -e --id Python.Python.3.12 --accept-source-agreements --accept-package-agreements --silent
    set "PATH=%LOCALAPPDATA%\Programs\Python\Python312;%LOCALAPPDATA%\Programs\Python\Python312\Scripts;%PATH%"
  )
  where python >nul 2>&1
  if errorlevel 1 (
    echo Python abhi bhi nahi mila. Dobara setup.bat chalao.
    pause
    exit /b 1
  )
  python -m pip install --quiet aqtinstall
  if errorlevel 1 (
    echo aqtinstall install nahi hua. Dobara setup.bat chalao.
    pause
    exit /b 1
  )
  python -m aqt install-qt windows desktop 6.7.3 win64_msvc2019_64 -m qtwebengine qtwebchannel qtpositioning qtwebsockets qtsvg -O C:\Qt
  if errorlevel 1 (
    echo Qt install nahi hua. Dobara setup.bat chalao.
    pause
    exit /b 1
  )
)
if not exist "C:\Qt\6.7.3\msvc2019_64\bin\qmake.exe" (
  echo Qt abhi bhi nahi mila. Dobara setup.bat chalao.
  pause
  exit /b 1
)
echo [4/5] Qt 6.7.3 mil gaya.

REM --- 5. Build + Qt files copy ---
echo.
echo [5/5] Sab kuch ready hai. Ab app build ho raha hai, kuch minute lagenge...
call build_core.bat
if errorlevel 1 (
  echo.
  echo BUILD FAIL ho gaya. Upar wala error dekho aur copy karke bhej do.
  pause
  exit /b 1
)
echo.
echo Qt files copy ho rahi hain...
"C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe" build\COREVEX.exe
if errorlevel 1 (
  echo.
  echo windeployqt FAIL ho gaya. Upar wala error dekho aur copy karke bhej do.
  pause
  exit /b 1
)
echo.
echo ================================================
echo  SETUP DONE. Ab app chalane ke liye run.bat
echo  par double-click karo.
echo ================================================
pause
