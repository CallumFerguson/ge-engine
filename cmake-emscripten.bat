@echo off
setlocal enabledelayedexpansion

REM Get the directory of the batch file
set "batch_dir=%~dp0"

REM Load environment variables from .env file
for /f "tokens=1,* delims==" %%a in ('type "%batch_dir%.env"') do (
    set "%%a=%%b"
)

if "!emscripten_cmake_path!"=="" (
    echo emscripten_cmake_path is not set in the .env file
    exit /b 1
)

REM Check if the input contains "--build"
echo %* | find "--build" > nul
if errorlevel 1 (
    REM Input does not contain "--build", so prefix "emcmake"
    set command=emcmake "!emscripten_cmake_path!" %*
) else (
    REM Input contains "--build", don't prefix "emcmake"
    set command="!emscripten_cmake_path!" %*
)

REM Execute the command
%command%
