@echo off
REM ============================================
REM  SeuLex + SeuYacc Build Script for MSVC
REM  Run from: x64 Native Tools Command Prompt
REM  Usage: build_msvc.bat
REM ============================================

pushd "%~dp0"

echo.
echo ========================================
echo   Building SeuLex (Lexical Analyzer Generator)
echo ========================================

cl /std:c++17 /EHsc /W3 ^
    /I shared /I seulex /I seuyacc ^
    /Fe:SeuLex.exe ^
    seulex\SeuLex.cpp ^
    seulex\LexParser.cpp ^
    seulex\REProcessor.cpp ^
    seulex\NFABuilder.cpp ^
    seulex\DFABuilder.cpp ^
    seulex\DFAMinimizer.cpp ^
    seulex\LexCodeGen.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: SeuLex build failed!
    popd
    exit /b 1
)

echo.
echo ========================================
echo   Building SeuYacc (Parser Generator)
echo ========================================

cl /std:c++17 /EHsc /W3 ^
    /I shared /I seulex /I seuyacc ^
    /Fe:SeuYacc.exe ^
    seuyacc\SeuYacc.cpp ^
    seuyacc\GrammarParser.cpp ^
    seuyacc\DFAGenerator.cpp ^
    seuyacc\Emitter.cpp

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: SeuYacc build failed!
    popd
    exit /b 1
)

echo.
echo ========================================
echo   Build OK! Files:
echo ========================================
dir SeuLex.exe SeuYacc.exe 2>nul

popd
