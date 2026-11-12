@echo off
REM ============================================================================
REM COREVEX — run.bat (har baar app chalane ke liye, sirf is par double-click karo)
REM ----------------------------------------------------------------------------
REM Agar app abhi tak build nahi hua hai to ye pehle setup.bat chala deta hai
REM (jo sab kuch install karke build karta hai), phir app kholta hai.
REM Agar app pehle se build hua hai to seedha khul jata hai, dobara build nahi hota.
REM ============================================================================
cd /d %~dp0
if not exist build\COREVEX.exe (
  echo App abhi build nahi hua hai. Pehle setup chala raha hun...
  echo.
  call setup.bat
  if errorlevel 1 (
    pause
    exit /b 1
  )
)
echo COREVEX khul raha hai...
start "" "build\COREVEX.exe"
