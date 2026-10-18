@echo off
setlocal enabledelayedexpansion
echo ========================================================
echo   Building Hammer Script Compiler and SDK with MSYS2
echo ========================================================

set PATH=C:\msys64\ucrt64\bin;%PATH%

set GPP=C:\msys64\ucrt64\bin\g++.exe
if not exist "%GPP%" (
    set GPP=C:\msys64\mingw64\bin\g++.exe
)
if not exist "%GPP%" (
    set GPP=g++
)

echo Using C++ Compiler: %GPP%

if not exist "bin" mkdir bin

"%GPP%" -std=c++17 -O3 -Wall -static ^
    src\token.cpp ^
    src\lexer.cpp ^
    src\diagnostics.cpp ^
    src\value.cpp ^
    src\environment.cpp ^
    src\stdlib.cpp ^
    src\hamfs.cpp ^
    src\asock.cpp ^
    src\hmripa.cpp ^
    src\hamsys.cpp ^
    src\hmshell.cpp ^
    src\hmcanvas.cpp ^
    src\lib_loader.cpp ^
    src\parser.cpp ^
    src\interpreter.cpp ^
    src\compiler.cpp ^
    src\module_manager.cpp ^
    src\modules\hamhook.cpp ^
    src\modules\hamc.cpp ^
    src\main.cpp ^
    -I include ^
    -lwininet -lws2_32 -lwinmm ^
    -o bin\hammer.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Check compiler messages above.
    echo.
    exit /b 1
)

echo.
echo [SUCCESS] Hammer Script CLI and Engine built successfully!
echo Binary: bin\hammer.exe

:: Create hamc.exe and hamhook.exe executables
copy /y bin\hammer.exe bin\hamc.exe >nul
copy /y bin\hammer.exe bin\hamhook.exe >nul
echo Generated standalone tools:
echo   - bin\hamc.exe     (C++ Transpiler CLI)
echo   - bin\hamhook.exe  (Dedicated Standalone PE Compiler CLI)

echo.
echo ========================================================
echo   Packaging Public Distribution SDK into 'Build\hammer-sdk'
echo ========================================================

set DIST=Build\hammer-sdk
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%DIST%"
mkdir "%DIST%\bin"
mkdir "%DIST%\lib loader"
mkdir "%DIST%\lib loader\community"
mkdir "%DIST%\lib loader\monolithiiun"
mkdir "%DIST%\hammer library header"
mkdir "%DIST%\stdlib"
mkdir "%DIST%\docs"

:: Copy binaries
copy /y bin\hammer.exe "%DIST%\bin\" >nul
copy /y bin\hamc.exe "%DIST%\bin\" >nul
copy /y bin\hamhook.exe "%DIST%\bin\" >nul

:: Copy lib loader files
copy /y "Lib loader\WARNING_SECURITY.txt" "%DIST%\lib loader\" >nul
if exist "Lib loader\community" (
    xcopy /e /i /y "Lib loader\community" "%DIST%\lib loader\community" >nul
)
if exist "Lib loader\monolithiiun" (
    xcopy /e /i /y "Lib loader\monolithiiun" "%DIST%\lib loader\monolithiiun" >nul
)

:: Copy library headers
xcopy /e /i /y "hammer library header\*.lib" "%DIST%\hammer library header\" >nul

:: Copy stdlib
xcopy /e /i /y "stdlib\*.hmscp" "%DIST%\stdlib\" >nul

:: Copy docs
xcopy /e /i /y "docs\*" "%DIST%\docs\" >nul

:: Copy root metadata files
copy /y "LICENSE" "%DIST%\" >nul
copy /y "README.md" "%DIST%\" >nul

echo.
echo [SDK READY] Public distribution packaged cleanly in:
echo   %DIST%\
echo.
endlocal
