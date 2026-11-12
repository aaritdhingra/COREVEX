@echo off
REM ============================================================================
REM SECTION — ADS TEST (build se pehle, alag command, Qt nahi chahiye)
REM ----------------------------------------------------------------------------
REM Ye test sirf ADS logic (Trie/Hash/Heap/Graph/AVL/Index/BM25) check karta
REM hai. Isme Qt, WebEngine ya full app build nahi chahiye — sirf MSVC cl.
REM Chalane ka tareeka: Developer Command Prompt se, browser folder me:
REM   test_ads.bat
REM Output me "ALL ADS TESTS PASSED" aana chahiye. Koi FAIL aaye to wahi
REM rukna hai, build aage nahi badhana.
REM ============================================================================
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
cd /d %~dp0
cl /std:c++20 /EHsc /I src tests\unit\test_ads.cpp /Fe:test_ads.exe
if errorlevel 1 exit /b 1
test_ads.exe
if errorlevel 1 exit /b 1
echo TEST_OK
